// Copyright (c) 1999, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// ---
// Revamped and reorganized by Craig Silverstein
//
// This file contains the implementation of all our command line flags
// stuff.  Here's how everything fits together
//
// * FlagRegistry owns CommandLineFlags owns FlagValue.
// * FlagSaver holds a FlagRegistry (saves it at construct time,
//     restores it at destroy time).
// * CommandLineFlagParser lives outside that hierarchy, but works on
//     CommandLineFlags (modifying the FlagValues).
// * Free functions like SetCommandLineOption() work via one of the
//     above (such as CommandLineFlagParser).
//
// In more detail:
//
// -- The main classes that hold flag data:
//
// FlagValue holds the current value of a flag.  It's
// pseudo-templatized: every operation on a FlagValue is typed.  It
// also deals with storage-lifetime issues (so flag values don't go
// away in a destructor), which is why we need a whole class to hold a
// variable's value.
//
// CommandLineFlag is all the information about a single command-line
// flag.  It has a FlagValue for the flag's current value, but also
// the flag's name, type, etc.
//
// FlagRegistry is a collection of CommandLineFlags.  There's the
// global registry, which is where flags defined via DEFINE_foo()
// live.  But it's possible to define your own flag, manually, in a
// different registry you create.  (In practice, multiple registries
// are used only by FlagSaver).
//
// A given FlagValue is owned by exactly one CommandLineFlag.  A given
// CommandLineFlag is owned by exactly one FlagRegistry.  FlagRegistry
// has a lock; any operation that writes to a FlagValue or
// CommandLineFlag owned by that registry must acquire the
// FlagRegistry lock before doing so.
//
// --- Some other classes and free functions:
//
// CommandLineFlagInfo is a client-exposed version of CommandLineFlag.
// Once it's instantiated, it has no dependencies or relationships
// with any other part of this file.
//
// FlagRegisterer is the helper class used by the DEFINE_* macros to
// allow work to be done at global initialization time.
//
// CommandLineFlagParser is the class that reads from the commandline
// and instantiates flag values based on that.  It needs to poke into
// the innards of the FlagValue->CommandLineFlag->FlagRegistry class
// hierarchy to do that.  It's careful to acquire the FlagRegistry
// lock before doing any writing or other non-const actions.
//
// GetCommandLineOption is just a hook into registry routines to
// retrieve a flag based on its name.  SetCommandLineOption, on the
// other hand, hooks into CommandLineFlagParser.  Other API functions
// are, similarly, mostly hooks into the functionality described above.

#include "config.h"
#include "gflags/gflags.h"

#include <cassert>
#include <cctype>
#include <cerrno>
#if defined(HAVE_FNMATCH_H)
#  include <fnmatch.h>
#elif defined(HAVE_SHLWAPI_H)
#  define NO_SHLWAPI_ISOS
#  include <shlwapi.h>
#endif
#include <cstdarg> // For va_list and related operations
#include <cstdio>
#include <cstring>

#include <algorithm>
#include <map>
#include <string>
#include <utility>     // for pair<>
#include <vector>

#include "mutex.h"
#include "util.h"

using namespace MUTEX_NAMESPACE;


// Special flags, type 1: the 'recursive' flags.  They set another flag's val.
DEFINE_string(flagfile,   "", "load flags from file");
DEFINE_string(fromenv,    "", "set flags from the environment"
                              " [use 'export FLAGS_flag1=value']");
DEFINE_string(tryfromenv, "", "set flags from the environment if present");

// Special flags, type 2: the 'parsing' flags.  They modify how we parse.
DEFINE_string(undefok, "", "comma-separated list of flag names that it is okay to specify "
                           "on the command line even if the program does not define a flag "
                           "with that name.  IMPORTANT: flags in this list that have "
                           "arguments MUST use the flag=value format");

namespace GFLAGS_NAMESPACE {

using std::map;
using std::pair;
using std::sort;
using std::string;
using std::vector;

// This is used by the unittest to test error-exit code
void GFLAGS_DLL_DECL (*gflags_exitfunc)(int) = &exit;  // from stdlib.h


// The help message indicating that the commandline flag has been
// 'stripped'. It will not show up when doing "-help" and its
// variants. The flag is stripped if STRIP_FLAG_HELP is set to 1
// before including base/gflags.h

// This is used by this file, and also in gflags_reporting.cc
const char kStrippedFlagHelp[] = "\001\002\003\004 (unknown) \004\003\002\001";

namespace {

// There are also 'reporting' flags, in gflags_reporting.cc.

static const char kError[] = "ERROR: ";

// Indicates that undefined options are to be ignored.
// Enables deferred processing of flags in dynamically loaded libraries.
static bool allow_command_line_reparsing = false;

static bool logging_is_probably_set_up = false;

// This is a 'prototype' validate-function.  'Real' validate
// functions, take a flag-value as an argument: ValidateFn(bool) or
// ValidateFn(uint64).  However, for easier storage, we strip off this
// argument and then restore it when actually calling the function on
// a flag value.
typedef bool (*ValidateFnProto)();

// Whether we should die when reporting an error.
enum DieWhenReporting { DIE, DO_NOT_DIE };

// Report Error and exit if requested.
static void ReportError(DieWhenReporting should_die, const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
  fflush(stderr);   // should be unnecessary, but cygwin's rxvt buffers stderr
  if (should_die == DIE) gflags_exitfunc(1);
}


// --------------------------------------------------------------------
// FlagValue
//    This represent the value a single flag might have.  The major
//    functionality is to convert from a string to an object of a
//    given type, and back.  Thread-compatible.
// --------------------------------------------------------------------
/*
  FlagValue 代表单个 flag 可能具有的值
  那扩展, 应该也得在这添加
  就是将字符串数值转换为一个给定的类型. wocao! 这不就是我一直想要的吗???
  搞搞搞. 我相信应该不会是 switch 或一堆 if else 激动激动激动
 */
class CommandLineFlag;
class FlagValue {
 public:
  enum ValueType {
    FV_BOOL = 0,
    FV_INT32 = 1,
    FV_UINT32 = 2,
    FV_INT64 = 3,
    FV_UINT64 = 4,
    FV_DOUBLE = 5,
    FV_STRING = 6,
    FV_MAX_INDEX = 6,
  };

  /**
   *  ooo 具体类型是从 DEFINE_xxx 中定义的, 害, 有些不太一样了哈哈
   *
   * @param [in]  valbuf  存储的类型值的指针
   * @param [in]  transfer_ownership_of_value 应该适合 terrace automem 中的一样表示是否要独占该值的所有权. 释放的时候, 有所有权才可以释放, 没有所有权就不能释放, 只当做引
   */
  template <typename FlagType>
  FlagValue(FlagType* valbuf, bool transfer_ownership_of_value);
  ~FlagValue();

  bool ParseFrom(const char* spec);
  string ToString() const;

  /**
   *     这份返回的事类型枚举
   */
  ValueType Type() const { return static_cast<ValueType>(type_); }

 private:
  friend class CommandLineFlag;  // for many things, including Validate()
  friend class GFLAGS_NAMESPACE::FlagSaverImpl;  // calls New()
  friend class FlagRegistry;     // checks value_buffer_ for flags_by_ptr_ map
  template <typename T> friend T GetFromEnv(const char*, T);
  friend bool TryParseLocked(const CommandLineFlag*, FlagValue*,
                             const char*, string*);  // for New(), CopyFrom()

  template <typename FlagType>
  struct FlagValueTraits;

  const char* TypeName() const;
  bool Equal(const FlagValue& x) const;
  FlagValue* New() const;   // creates a new one with default value
  void CopyFrom(const FlagValue& x);

  // Calls the given validate-fn on value_buffer_, and returns
  // whatever it returns.  But first casts validate_fn_proto to a
  // function that takes our value as an argument (eg void
  // (*validate_fn)(bool) for a bool flag).
  bool Validate(const char* flagname, ValidateFnProto validate_fn_proto) const;

  void* const value_buffer_;          // points to the buffer holding our data
  const int8 type_;                   // how to interpret value_
  const bool owns_value_;             // whether to free value on destruct

  FlagValue(const FlagValue&);   // no copying!
  void operator=(const FlagValue&);
};

/* 哦都是发生在编译时间, ooo 我也可以这么搞
在程序内写好注册, 配置文件中, 通过传入指定字段来进行传递
可以想一想 */
// Map the given C++ type to a value of the ValueType enum at compile time.
#define DEFINE_FLAG_TRAITS(type, value)        \
  template <>                                  \
  struct FlagValue::FlagValueTraits<type> {    \
    static const ValueType kValueType = value; \
  }

// Define full template specializations of the FlagValueTraits template
// for all supported flag types.
DEFINE_FLAG_TRAITS(bool, FV_BOOL);
DEFINE_FLAG_TRAITS(int32, FV_INT32);
DEFINE_FLAG_TRAITS(uint32, FV_UINT32);
DEFINE_FLAG_TRAITS(int64, FV_INT64);
DEFINE_FLAG_TRAITS(uint64, FV_UINT64);
DEFINE_FLAG_TRAITS(double, FV_DOUBLE);
DEFINE_FLAG_TRAITS(std::string, FV_STRING);

#undef DEFINE_FLAG_TRAITS


// This could be a templated method of FlagValue, but doing so adds to the
// size of the .o.  Since there's no type-safety here anyway, macro is ok.
#define VALUE_AS(type)  *reinterpret_cast<type*>(value_buffer_)
#define OTHER_VALUE_AS(fv, type)  *reinterpret_cast<type*>(fv.value_buffer_)
#define SET_VALUE_AS(type, value)  VALUE_AS(type) = (value)

template <typename FlagType>
FlagValue::FlagValue(FlagType* valbuf,
                     bool transfer_ownership_of_value)
    : value_buffer_(valbuf),
      /* 通过 traits 在编译其获得一个类型值 */
      type_(FlagValueTraits<FlagType>::kValueType),
      owns_value_(transfer_ownership_of_value) {
}

FlagValue::~FlagValue() {
  if (!owns_value_) { /* 并非自己的所有权, 就不能释放, 表明当前只是引用 */
    return;
  }

  /* all right */
  switch (type_) {
    case FV_BOOL: delete reinterpret_cast<bool*>(value_buffer_); break;
    case FV_INT32: delete reinterpret_cast<int32*>(value_buffer_); break;
    case FV_UINT32: delete reinterpret_cast<uint32*>(value_buffer_); break;
    case FV_INT64: delete reinterpret_cast<int64*>(value_buffer_); break;
    case FV_UINT64: delete reinterpret_cast<uint64*>(value_buffer_); break;
    case FV_DOUBLE: delete reinterpret_cast<double*>(value_buffer_); break;
    case FV_STRING: delete reinterpret_cast<string*>(value_buffer_); break;
  }
}


/**
 *     解析传入的命令行参数, 能成功解析就保存传入的值, 不能解析就返回 false
 *
 * @param [in]  value   待解析的值
 * @return     true 成功解析, false 解析失败
 */
bool FlagValue::ParseFrom(const char* value) {
  /* ooo 对于格式解析, 也是使用了 switch if else 的方式.
  我有些钻牛角尖了, switch if else 存在即合理, 完全消灭才是邪道
  要突破自己的思维定式! 有些问题这就是最好的解决方案
   */
  if (type_ == FV_BOOL) {
    /* bool 类型 可以用这么多表示 */
    const char* kTrue[] = { "1", "t", "true", "y", "yes" };
    const char* kFalse[] = { "0", "f", "false", "n", "no" };
    COMPILE_ASSERT(sizeof(kTrue) == sizeof(kFalse), true_false_equal);
    for (size_t i = 0; i < sizeof(kTrue)/sizeof(*kTrue); ++i) {
      /* C语言中判断字符串是否相等的函数，忽略大小写。s1和s2中的所有字母字符在比较之前都转换为小写。 */
      if (strcasecmp(value, kTrue[i]) == 0) {
        SET_VALUE_AS(bool, true); /* *reinterpret_cast<bool*>(value_buffer_) = (true) */
        return true;
      } else if (strcasecmp(value, kFalse[i]) == 0) {
        SET_VALUE_AS(bool, false);
        return true;
      }
    }
    return false;   // didn't match a legal input

  } else if (type_ == FV_STRING) {
    SET_VALUE_AS(string, value); /* *reinterpret_cast<string*>(value_buffer_) = (value) */
    return true;
  }

  /* 剩下就是对数值类型做专门处理了 */
  // OK, it's likely to be numeric, and we'll be using a strtoXXX method.
  if (value[0] == '\0')   // empty-string is only allowed for string type.
    return false;   /* 数值标记不能使用空字符串传递 */
  char* end;
  /* 也就是支持 16 进制和 10 进制的出传递, 16 进制用 0x 做前导
  但是不能使用 0 作为 8 进制的前导 */
  // Leading 0x puts us in base 16.  But leading 0 does not put us in base 8!
  // It caused too many bugs when we had that behavior.
  int base = 10;    // by default
  if (value[0] == '0' && (value[1] == 'x' || value[1] == 'X'))
    base = 16;
  errno = 0;

  switch (type_) {
    case FV_INT32: {
      const int64 r = strto64(value, &end, base);
      if (errno || end != value + strlen(value))  return false;  // bad parse
      if (static_cast<int32>(r) != r)  // worked, but number out of range
        return false;
      SET_VALUE_AS(int32, static_cast<int32>(r));
      return true;
    }
    case FV_UINT32: {
      while (*value == ' ') value++;
      if (*value == '-') return false;  // negative number
      const uint64 r = strtou64(value, &end, base);
      if (errno || end != value + strlen(value))  return false;  // bad parse
        if (static_cast<uint32>(r) != r)  // worked, but number out of range
        return false;
      SET_VALUE_AS(uint32, static_cast<uint32>(r));
      return true;
    }
    case FV_INT64: {
      const int64 r = strto64(value, &end, base);
      if (errno || end != value + strlen(value))  return false;  // bad parse
      SET_VALUE_AS(int64, r);
      return true;
    }
    case FV_UINT64: {
      while (*value == ' ') value++;
      if (*value == '-') return false;  // negative number
      const uint64 r = strtou64(value, &end, base);
      if (errno || end != value + strlen(value))  return false;  // bad parse
      SET_VALUE_AS(uint64, r);
      return true;
    }
    case FV_DOUBLE: {
      const double r = strtod(value, &end);
      if (errno || end != value + strlen(value))  return false;  // bad parse
      SET_VALUE_AS(double, r);
      return true;
    }
    default: {
      assert(false);  // unknown type
      return false;
    }
  }
}

/* 将传入的值以一个字符串的形式进行返回 */
string FlagValue::ToString() const {
  char intbuf[64];    // enough to hold even the biggest number
  switch (type_) {
    case FV_BOOL:
      // 判断当前缓冲区存的值是什么 以 true 和 false 为返回值
      // *reinterpret_cast<bool*>(value_buffer_)
      return VALUE_AS(bool) ? "true" : "false";
    case FV_INT32:
      /* 将可变参数 “…” 按照format的格式格式化为字符串，然后再将其拷贝至str中。
      就是将整数转为字符串 那为啥不直接用 to_string 不是更简单吗 */
      snprintf(intbuf, sizeof(intbuf), "%" PRId32, VALUE_AS(int32));
      return intbuf;
    case FV_UINT32:
      snprintf(intbuf, sizeof(intbuf), "%" PRIu32, VALUE_AS(uint32));
      return intbuf;
    case FV_INT64:
      snprintf(intbuf, sizeof(intbuf), "%" PRId64, VALUE_AS(int64));
      return intbuf;
    case FV_UINT64:
      snprintf(intbuf, sizeof(intbuf), "%" PRIu64, VALUE_AS(uint64));
      return intbuf;
    case FV_DOUBLE:
      // 可能是想控制一下输出格式
      snprintf(intbuf, sizeof(intbuf), "%.17g", VALUE_AS(double));
      return intbuf;
    case FV_STRING:
      return VALUE_AS(string);
    default:
      assert(false);
      return "";  // unknown type
  }
}

/* typedef bool (*ValidateFnProto)();
哎哟哟哟, 可以将一个函数指针从一个指针强转到另一个, 可以啊
reinterpret_cast 果然强大, 没有它不能转的!!! 可以可以, 可以考虑考虑
validate_fn_proto 应该是用户传过来的, 后面再看看
验证传入的值是不是符合用户的要求

传过来的函数指针是带参数的, 存储的时候是不带参数的, 我什么要这么搞???
这样就可以将所有不同类型的校验器存到一起!!! 不需要为不同类型的校验器存储不同的类型
wocao!!! 强啊, 函数指针说到底就还是一个指针!!!
六六六啊, 学习到了
*/
bool FlagValue::Validate(const char* flagname,
                         ValidateFnProto validate_fn_proto) const {
  switch (type_) {
    case FV_BOOL:
      return reinterpret_cast<bool (*)(const char*, bool)>(
          validate_fn_proto)(flagname, VALUE_AS(bool));
    case FV_INT32:
      return reinterpret_cast<bool (*)(const char*, int32)>(
          validate_fn_proto)(flagname, VALUE_AS(int32));
    case FV_UINT32:
      return reinterpret_cast<bool (*)(const char*, uint32)>(
          validate_fn_proto)(flagname, VALUE_AS(uint32));
    case FV_INT64:
      return reinterpret_cast<bool (*)(const char*, int64)>(
          validate_fn_proto)(flagname, VALUE_AS(int64));
    case FV_UINT64:
      return reinterpret_cast<bool (*)(const char*, uint64)>(
          validate_fn_proto)(flagname, VALUE_AS(uint64));
    case FV_DOUBLE:
      return reinterpret_cast<bool (*)(const char*, double)>(
          validate_fn_proto)(flagname, VALUE_AS(double));
    case FV_STRING:
      return reinterpret_cast<bool (*)(const char*, const string&)>(
          validate_fn_proto)(flagname, VALUE_AS(string));
    default:
      assert(false);  // unknown type
      return false;
  }
}

/* 获取类型的中文名字
这种方式好像比 map 节省很多啊, 可以考虑考虑用一下
 */
const char* FlagValue::TypeName() const {
  static const char types[] =
      "bool\0xx"
      "int32\0x"
      "uint32\0"
      "int64\0x"
      "uint64\0"
      "double\0"
      "string";
  /* 这里应该就是一个字符串指针, 字符串数组
  对就是一个字符串 */
  if (type_ > FV_MAX_INDEX) {
    assert(false);
    return "";
  }
  // Directly indexing the strings in the 'types' string, each of them is 7 bytes long.
  return &types[type_ * 7]; /* 因为每个类型 7 字节长, 所以这里 * 7 */
}

/**
 *     就是重载的 operator==
 * 比较当前对象和给定的对象是否一致
 */
bool FlagValue::Equal(const FlagValue& x) const {
  if (type_ != x.type_)
    return false;
  switch (type_) {
    case FV_BOOL:   return VALUE_AS(bool) == OTHER_VALUE_AS(x, bool);
    case FV_INT32:  return VALUE_AS(int32) == OTHER_VALUE_AS(x, int32);
    case FV_UINT32: return VALUE_AS(uint32) == OTHER_VALUE_AS(x, uint32);
    case FV_INT64:  return VALUE_AS(int64) == OTHER_VALUE_AS(x, int64);
    case FV_UINT64: return VALUE_AS(uint64) == OTHER_VALUE_AS(x, uint64);
    case FV_DOUBLE: return VALUE_AS(double) == OTHER_VALUE_AS(x, double);
    case FV_STRING: return VALUE_AS(string) == OTHER_VALUE_AS(x, string);
    default: assert(false); return false;  // unknown type
  }
}

/* 依据类型创建爱你一个变量 */
FlagValue* FlagValue::New() const {
  switch (type_) {
    case FV_BOOL:   return new FlagValue(new bool(false), true);
    case FV_INT32:  return new FlagValue(new int32(0), true);
    case FV_UINT32: return new FlagValue(new uint32(0), true);
    case FV_INT64:  return new FlagValue(new int64(0), true);
    case FV_UINT64: return new FlagValue(new uint64(0), true);
    case FV_DOUBLE: return new FlagValue(new double(0.0), true);
    case FV_STRING: return new FlagValue(new string, true);
    default: assert(false); return NULL;  // unknown type
  }
}

/*
  就是 operator= 或者 拷贝构造, 只不过这里使用成员方法的方式进行了替代
 */
void FlagValue::CopyFrom(const FlagValue& x) {
  assert(type_ == x.type_);
  switch (type_) {
    case FV_BOOL:   SET_VALUE_AS(bool, OTHER_VALUE_AS(x, bool));      break;
    case FV_INT32:  SET_VALUE_AS(int32, OTHER_VALUE_AS(x, int32));    break;
    case FV_UINT32: SET_VALUE_AS(uint32, OTHER_VALUE_AS(x, uint32));  break;
    case FV_INT64:  SET_VALUE_AS(int64, OTHER_VALUE_AS(x, int64));    break;
    case FV_UINT64: SET_VALUE_AS(uint64, OTHER_VALUE_AS(x, uint64));  break;
    case FV_DOUBLE: SET_VALUE_AS(double, OTHER_VALUE_AS(x, double));  break;
    case FV_STRING: SET_VALUE_AS(string, OTHER_VALUE_AS(x, string));  break;
    default: assert(false);  // unknown type
  }
}

// --------------------------------------------------------------------
// CommandLineFlag
//    This represents a single flag, including its name, description,
//    default value, and current value.  Mostly this serves as a
//    struct, though it also knows how to register itself.
//       All CommandLineFlags are owned by a (exactly one)
//    FlagRegistry.  If you wish to modify fields in this class, you
//    should acquire the FlagRegistry lock for the registry that owns
//    this flag.
// --------------------------------------------------------------------
/*
  FlagValue 只存值, 而 CommandLineFlag 就是将整个用户定义的标记转为定义的这个类
  这样就将外部的输入统一起来的.
  对就应该是这样, 外部的输入一定要封装到我们自己的类中, 更好地控制, 而不要东一个西一点
  FlagRegistry 应该就是一个单例, 一个全局注册表
  修改的话要拿到这个标记的锁? 在注册表中是为每个标记都搞了一锁码? 研究研究
 */
class CommandLineFlag {
 public:
  // Note: we take over memory-ownership of current_val and default_val.
  CommandLineFlag(const char* name, const char* help, const char* filename,
                  FlagValue* current_val, FlagValue* default_val);
  ~CommandLineFlag();

  const char* name() const { return name_; }
  const char* help() const { return help_; }
  const char* filename() const { return file_; }
  const char* CleanFileName() const;  // nixes irrelevant prefix such as homedir
  string current_value() const { return current_->ToString(); }
  string default_value() const { return defvalue_->ToString(); }
  const char* type_name() const { return defvalue_->TypeName(); }

  /* 返回没问题, 但是这个东西是从哪传入的来???? */
  ValidateFnProto validate_function() const { return validate_fn_proto_; }
  const void* flag_ptr() const { return current_->value_buffer_; }

  FlagValue::ValueType Type() const { return defvalue_->Type(); }

  void FillCommandLineFlagInfo(struct CommandLineFlagInfo* result);

  // If validate_fn_proto_ is non-NULL, calls it on value, returns result.
  bool Validate(const FlagValue& value) const;
  bool ValidateCurrent() const { return Validate(*current_); }
  bool Modified() const { return modified_; }

 private:
  // for SetFlagLocked() and setting flags_by_ptr_
  friend class FlagRegistry;
  friend class GFLAGS_NAMESPACE::FlagSaverImpl;  // for cloning the values
  // set validate_fn
  friend bool AddFlagValidator(const void*, ValidateFnProto);

  // This copies all the non-const members: modified, processed, defvalue, etc.
  void CopyFrom(const CommandLineFlag& src);

  void UpdateModifiedBit();

  const char* const name_;     // Flag name
  const char* const help_;     // Help message
  const char* const file_;     // Which file did this come from?
  bool modified_;              // Set after default assignment?
  FlagValue* defvalue_;        // Default value for flag
  FlagValue* current_;         // Current value for flag
  // This is a casted, 'generic' version of validate_fn, which actually
  // takes a flag-value as an arg (void (*validate_fn)(bool), say).
  // When we pass this to current_->Validate(), it will cast it back to
  // the proper type.  This may be NULL to mean we have no validate_fn.
  ValidateFnProto validate_fn_proto_;

  CommandLineFlag(const CommandLineFlag&);   // no copying!
  void operator=(const CommandLineFlag&);
};

CommandLineFlag::CommandLineFlag(const char* name, const char* help,
                                 const char* filename,
                                 FlagValue* current_val, FlagValue* default_val)
    : name_(name), help_(help), file_(filename), modified_(false),
      defvalue_(default_val), current_(current_val), validate_fn_proto_(NULL) {
}

/* 因为这是一个内部类, 内部类对彼此都是很了解的, 所以这里就直接
使用 delete 进行资源释放.
  如果是对外的接口, 我觉得还是用智能指针释放比较好, 否则
  我们无法判定如果用户传入时, 他传入的是栈还是堆地址 */
CommandLineFlag::~CommandLineFlag() {
  delete current_;
  delete defvalue_;
}

/* 这个没啥用 */
const char* CommandLineFlag::CleanFileName() const {
  // This function has been used to strip off a common prefix from
  // flag source file names. Because flags can be defined in different
  // shared libraries, there may not be a single common prefix.
  // Further, this functionality hasn't been active for many years.
  // Need a better way to produce more user friendly help output or
  // "anonymize" file paths in help output, respectively.
  // Follow issue at: https://github.com/gflags/gflags/issues/86
  return filename();
}

/**
 *  将命令行参数保存到  CommandLineFlagInfo  中
 *
 * @param [out] result    处理好的命令行参数
 */
void CommandLineFlag::FillCommandLineFlagInfo(
    CommandLineFlagInfo* result) {
  result->name = name();
  result->type = type_name();
  result->description = help();
  result->current_value = current_value();
  result->default_value = default_value();
  result->filename = CleanFileName();
  UpdateModifiedBit();
  result->is_default = !modified_;  /* 如果进行了一次修改, modified_ 就会是 true */
  result->has_validator_fn = validate_function() != NULL;
  result->flag_ptr = flag_ptr();    /* 这个是具体的当前值 */
}

/**
 *     更新 modified_ 的状态
 *
 *  就是为了防止直接使用 FLAGS_name 这种方式修改变量, 怎么防止的?
 */
void CommandLineFlag::UpdateModifiedBit() {
  // Update the "modified" bit in case somebody bypassed the
  // Flags API and wrote directly through the FLAGS_name variable.
  if (!modified_ && !current_->Equal(*defvalue_)) { /* 只要当前值和实际值不一样就认为是被修改了 */
    modified_ = true;
  }
}

/**
 *  不允许使用 operator= 来拷贝, 只能使用成员方法进行
 * @param [in]  src 要拷贝的变量
 */
void CommandLineFlag::CopyFrom(const CommandLineFlag& src) {
  // Note we only copy the non-const members; others are fixed at construct time
  if (modified_ != src.modified_) modified_ = src.modified_;
  if (!current_->Equal(*src.current_)) current_->CopyFrom(*src.current_);
  if (!defvalue_->Equal(*src.defvalue_)) defvalue_->CopyFrom(*src.defvalue_);
  if (validate_fn_proto_ != src.validate_fn_proto_)
    validate_fn_proto_ = src.validate_fn_proto_;
}

/**
 *      函数应该是有用户指定的, 用来校验结果传入的参数是否符合用户要求
 *
 * @param [in]  value   用户通过命令行传入的参数
 * @return     true ok false 不满足要求, 这个应该由用户自己提供方法来进行校验
 */
bool CommandLineFlag::Validate(const FlagValue& value) const {
  /* validate_function 这个是成员函数, 用来返回校验函数指针 */
  if (validate_function() == NULL)
    return true;
  else
    return value.Validate(name(), validate_function());
}


// --------------------------------------------------------------------
// FlagRegistry
//    A FlagRegistry singleton object holds all flag objects indexed
//    by their names so that if you know a flag's name (as a C
//    string), you can access or set it.  If the function is named
//    FooLocked(), you must own the registry lock before calling
//    the function; otherwise, you should *not* hold the lock, and
//    the function will acquire it itself if needed.
// --------------------------------------------------------------------

struct StringCmp {  // Used by the FlagRegistry map class to compare char*'s
  bool operator() (const char* s1, const char* s2) const {
    return (strcmp(s1, s2) < 0);
  }
};


/**
 *     这个是注册表, 提供下面一些功能:
 *  (1) 进行 Flag 的注册
 *  (2) 解析命令行传入的参数, 并将其存储到已有的 CommandLineFlag 中
 *
 * @param [in]  
 * @param [out] 
 * @return     无
 */
class FlagRegistry {
 public:
  /* 这样防止不了用户通过默认构造来定义对象, 应该 gflags 最开始的时候还没有 c++11, delete default 都还没出来. 对应该就是还没有 11. 第一版 gflags 是 2008 年的, 11 标准还得几年 */
  FlagRegistry() {
  }
  ~FlagRegistry() {
    // Not using STLDeleteElements as that resides in util and this
    // class is base.
    // 可以把 end() 放到初始化中, 这样就不用每次都再调用一次
    for (FlagMap::iterator p = flags_.begin(), e = flags_.end(); p != e; ++p) {
      CommandLineFlag* flag = p->second;
      delete flag;
    }
  }

  static void DeleteGlobalRegistry() {
    delete global_registry_;
    global_registry_ = NULL;
  }

  // Store a flag in this registry.  Takes ownership of the given pointer.
  void RegisterFlag(CommandLineFlag* flag);

  void Lock() { lock_.Lock(); }
  void Unlock() { lock_.Unlock(); }

  // Returns the flag object for the specified name, or NULL if not found.
  CommandLineFlag* FindFlagLocked(const char* name);

  // Returns the flag object whose current-value is stored at flag_ptr.
  // That is, for whom current_->value_buffer_ == flag_ptr
  CommandLineFlag* FindFlagViaPtrLocked(const void* flag_ptr);

  // A fancier form of FindFlag that works correctly if name is of the
  // form flag=value.  In that case, we set key to point to flag, and
  // modify v to point to the value (if present), and return the flag
  // with the given name.  If the flag does not exist, returns NULL
  // and sets error_message.
  CommandLineFlag* SplitArgumentLocked(const char* argument,
                                       string* key, const char** v,
                                       string* error_message);

  // Set the value of a flag.  If the flag was successfully set to
  // value, set msg to indicate the new flag-value, and return true.
  // Otherwise, set msg to indicate the error, leave flag unchanged,
  // and return false.  msg can be NULL.
  bool SetFlagLocked(CommandLineFlag* flag, const char* value,
                     FlagSettingMode set_mode, string* msg);

  static FlagRegistry* GlobalRegistry();   // returns a singleton registry

 private:
  friend class GFLAGS_NAMESPACE::FlagSaverImpl;  // reads all the flags in order to copy them
  friend class CommandLineFlagParser;    // for ValidateUnmodifiedFlags
  friend void GFLAGS_NAMESPACE::GetAllFlags(vector<CommandLineFlagInfo>*);

  // The map from name to flag, for FindFlagLocked().
  typedef map<const char*, CommandLineFlag*, StringCmp> FlagMap;
  typedef FlagMap::iterator FlagIterator;
  typedef FlagMap::const_iterator FlagConstIterator;
  FlagMap flags_;   /* 和我搞的差不多, 都是 map, 只不过我更喜欢 unordered_map
                      因为注册只需要注册一次, 不会进行大量检索, map 空间更小一些 */
                    /*!< 存储 Flag 名字和 CommandLineFlag* */

  // The map from current-value pointer to flag, fo FindFlagViaPtrLocked().
  typedef map<const void*, CommandLineFlag*> FlagPtrMap;
  FlagPtrMap flags_by_ptr_; /* 再存储一个由值找到 Flag 对象的映射 */

  static FlagRegistry* global_registry_;   // a singleton registry

  Mutex lock_;

  /* 这个的是实现来???? */
  static void InitGlobalRegistry();

  // Disallow
  FlagRegistry(const FlagRegistry&);
  FlagRegistry& operator=(const FlagRegistry&);
};

/* 这个就是 std::unique_lock<Mutex> */
class FlagRegistryLock {
 public:
  explicit FlagRegistryLock(FlagRegistry* fr) : fr_(fr) { fr_->Lock(); }
  ~FlagRegistryLock() { fr_->Unlock(); }
 private:
  FlagRegistry *const fr_;
};

/**
 *     将一个命令行参数, 注册到注册表中, 注册就是将相应的对象保存到
 * map 中的过程
 *  线程安全
 * @param [in]  flag    待注册 flag
 */
void FlagRegistry::RegisterFlag(CommandLineFlag* flag) {
  Lock(); /* 保证线程安全 */
  pair<FlagIterator, bool> ins =
    flags_.insert(pair<const char*, CommandLineFlag*>(flag->name(), flag));
  if (ins.second == false) {   // means the name was already in the map
    /* 重复插入, 不同文件中, 也只允许有一个 Flag, 所有 Flag 的名字必须唯一!!! */
    if (strcmp(ins.first->second->filename(), flag->filename()) != 0) {
      ReportError(DIE, "ERROR: flag '%s' was defined more than once "
                  "(in files '%s' and '%s').\n",
                  flag->name(),
                  ins.first->second->filename(),
                  flag->filename());
    } else {
      ReportError(DIE, "ERROR: something wrong with flag '%s' in file '%s'.  "
                  "One possibility: file '%s' is being linked both statically "
                  "and dynamically into this executable.\n",
                  flag->name(),
                  flag->filename(), flag->filename());
    }
  }
  // Also add to the flags_by_ptr_ map.
  flags_by_ptr_[flag->current_->value_buffer_] = flag;
  Unlock();
}

/**
 *     在有锁的前提下, 通过名字获取 Flag 对象指针
 *
 * @param [in]  name    待获取 flag 的名字
 * @return     Flag 对象指针
 */
CommandLineFlag* FlagRegistry::FindFlagLocked(const char* name) {
  FlagConstIterator i = flags_.find(name);
  if (i == flags_.end()) {
    // If the name has dashes in it, try again after replacing with
    // underscores.
    // 为什么有 - 要替换为 _ 再试一次. 存储的时候统一会将 - 变为 _ 吗? 没发现哇
    if (strchr(name, '-') == NULL) return NULL;
    string name_rep = name;
    std::replace(name_rep.begin(), name_rep.end(), '-', '_');
    return FindFlagLocked(name_rep.c_str());
  } else {
    return i->second;
  }
}

/**
 *     同 FindFlagLocked 功能类似, 只不过这个是通过值来找到 Flag 对象指针
 *  flags_by_ptr_ <void*, CommandLineFlag>
 * @param [in]  flag_ptr    flag 值
 * @return     对应的 CommandLineFlag 指针
 */
CommandLineFlag* FlagRegistry::FindFlagViaPtrLocked(const void* flag_ptr) {
  FlagPtrMap::const_iterator i = flags_by_ptr_.find(flag_ptr);
  if (i == flags_by_ptr_.end()) {
    return NULL;
  } else {
    return i->second;
  }
}

/**
 *     看名字是分割字符串
 * 这个应该是将 命令行 的传参, 一个一个进行解析, 解析一个就跳过一个
 *
 *  分离出 flag 名字之后, 先检查注册表中是否有, 有就获取值
 *  如果解析的名字不存在在注册表中, 就检查这个名字是不是 noxxx 的形式
 *  若果是, 再看一看是不是 bool 类型, gflag 允许 noxxx 的形式作为 bool 标记
 *  的否! noxxx 仅支持 bool 标记
 *
 * @param [in]   arg    还剩下的参数
 * @param [out]  key    解析得到的 flag 名字, 没匹配到就保存原值
 * @param [out]  v      解析得到传入的 flag 值, 可能返回 NULL, 说明当前参数中没有 =
 * @param [out]  error_message   存在的错误信息
 * @return     注册表中如果存在该 flag, 就将相应的 flag 对象拿出来
 */
CommandLineFlag* FlagRegistry::SplitArgumentLocked(const char* arg,
                                                   string* key,
                                                   const char** v,
                                                   string* error_message) {
  // Find the flag object for this option
  const char* flag_name;
  const char* value = strchr(arg, '='); /* 为在一个串中查找给定字符的第一个匹配之处 */
  if (value == NULL) {  /* 可以使用 = 也可以不使用 = 进行赋值 */
    key->assign(arg);   /* 没有 = 获得的是什么就是什么 */
    *v = NULL;
  } else {
    // Strip out the "=value" portion from arg
    key->assign(arg, value-arg);  /* 有等号, 就将等号左侧赋给 key, 右侧给 value */
    *v = ++value;    // advance past the '='  去掉 =
  }
  flag_name = key->c_str();

  CommandLineFlag* flag = FindFlagLocked(flag_name);

  if (flag == NULL) {
    /* 没有找到的话, 就返回错误信息, 下面这一段代码就是在整理错误信息 */
    // If we can't find the flag-name, then we should return an error.
    // The one exception is if 1) the flag-name is 'nox', 2) there
    // exists a flag named 'x', and 3) 'x' is a boolean flag.
    // In that case, we want to return flag 'x'.

    /* 这里就看出来了, 允许传入一个 noxxx 的标记, 作为 xxx 标记的反 */
    if (!(flag_name[0] == 'n' && flag_name[1] == 'o')) {
      // flag-name is not 'nox', so we're not in the exception case.
      *error_message = StringPrintf("%sunknown command line flag '%s'\n",
                                    kError, key->c_str());
      return NULL;
    }

    /* 到这里就是找到了一个 noxxx 标记 */
    flag = FindFlagLocked(flag_name+2);
    if (flag == NULL) {
      /* 去掉 no 的话要是还没找到, 就是真的没有了 */
      // No flag named 'x' exists, so we're not in the exception case.
      *error_message = StringPrintf("%sunknown command line flag '%s'\n",
                                    kError, key->c_str());
      return NULL;
    }

    /* 哦吼, noxxx 标记只允许在 bool 值的情况下使用!!! */
    if (flag->Type() != FlagValue::FV_BOOL) {
      // 'x' exists but is not boolean, so we're not in the exception case.
      *error_message = StringPrintf(
          "%sboolean value (%s) specified for %s command line flag\n",
          kError, key->c_str(), flag->type_name());
      return NULL;
    }
    // We're in the exception case!
    // Make up a fake value to replace the "no" we stripped out
    key->assign(flag_name+2);   // the name without the "no"
    *v = "0";
  }

  // Assign a value if this is a boolean flag
  if (*v == NULL && flag->Type() == FlagValue::FV_BOOL) {
    *v = "1";    // the --nox case was already handled, so this is the --x case
  }

  return flag;
}

/**
 *     将获得的命令行参数进行解析, 看传入的值能不能解析为定义时指定的类型,
 * 如同能解析, 还会接着使用用户传入的校验函数校验参数的数值范围. 不能解析就会返回
 * 相关错误信息
 *
 *    校验数据类型  校验数据范围
 *
 * @param [in]  flag            包装好的命令行参数
 * @param [in out]  flag_value  解析成功的 flag 值
 * @param [in]  value           待解析的命令行参数
 * @param [in]  msg             保存相关信息
 * @return     true 能正常解析, false 不能正常解析
 */
bool TryParseLocked(const CommandLineFlag* flag, FlagValue* flag_value,
                    const char* value, string* msg) {
  // Use tenative_value, not flag_value, until we know value is valid.
  /*  从一个随机值开始, 直到找到一个确定的值, 可以看做是一个临时变量 */
  FlagValue* tentative_value = flag_value->New();
  if (!tentative_value->ParseFrom(value)) { /* 不能解析的就返回错误信息 */
    if (msg) {
      StringAppendF(msg,
                    "%sillegal value '%s' specified for %s flag '%s'\n",
                    kError, value,
                    flag->type_name(), flag->name());
    }
    delete tentative_value;
    return false;

  /* 如果有用户传入的校验函数, 会对传入值进行一次校验, 输入不对仍然会报错,
  这个校验函数可以搞一搞 */
  } else if (!flag->Validate(*tentative_value)) { /* 同样, 如果校验函数失败, 就返回错误信息 */
    if (msg) {
      StringAppendF(msg,
          "%sfailed validation of new value '%s' for flag '%s'\n",
          kError, tentative_value->ToString().c_str(),
          flag->name());
    }
    delete tentative_value;
    return false;
  } else {
    /* 一切 ok, 能解析为指定类型, 就把解析得到的数据保存到 flag_value 中 */
    flag_value->CopyFrom(*tentative_value);
    if (msg) {
      StringAppendF(msg, "%s set to %s\n",
                    flag->name(), flag_value->ToString().c_str());
    }
    delete tentative_value;
    return true;
  }
}

/**
 *  按照指定的设置模式进行值的设置
 *  
 * @param [in out]  flag      命令行 Flag 对象
 * @param [in]      value     命令行传入的参数
 * @param [in]      set_mode  设置值的模式
 * @param [in]      msg       保存相关信息
 * @return     true 成功设置, false 设置失败
 */
bool FlagRegistry::SetFlagLocked(CommandLineFlag* flag,
                                 const char* value,
                                 FlagSettingMode set_mode,
                                 string* msg) {
  flag->UpdateModifiedBit();  /* 要设置 flag 的值了, 肯定是要进行修改 */
  switch (set_mode) {
    /* o FlagSettingMode 这个枚举的作用好像就是设置不同情况下, 进行更新 */
    case SET_FLAGS_VALUE: {   /* 正常修改值, 无论什么情况下都会正常gengxin */
      // set or modify the flag's value
      if (!TryParseLocked(flag, flag->current_, value, msg))
        return false;
      flag->modified_ = true;   /* 标记值被修改过 */
      break;
    }
    case SET_FLAG_IF_DEFAULT: {
      // set the flag's value, but only if it hasn't been set by someone else
      if (!flag->modified_) {   /* 当没有被修改过时, 才会进行值的设定 */
        if (!TryParseLocked(flag, flag->current_, value, msg))
          return false;
        flag->modified_ = true;
      } else {
        *msg = StringPrintf("%s set to %s",
                            flag->name(), flag->current_value().c_str());
      }
      break;
    }
    case SET_FLAGS_DEFAULT: {
      // modify the flag's default-value  无论何时都会设置为默认值
      if (!TryParseLocked(flag, flag->defvalue_, value, msg))
        return false;
      if (!flag->modified_) {
        // Need to set both defvalue *and* current, in this case
        TryParseLocked(flag, flag->current_, value, NULL);
      }
      break;
    }
    default: {
      // unknown set_mode
      assert(false);
      return false;
    }
  }

  return true;
}

// Get the singleton FlagRegistry object
FlagRegistry* FlagRegistry::global_registry_ = NULL;

/**
 *  这个就是正规的产出对象
 * @return     单例对象
 */
FlagRegistry* FlagRegistry::GlobalRegistry() {
  static Mutex lock(Mutex::LINKER_INITIALIZED);
  MutexLock acquire_lock(&lock);
  if (!global_registry_) {
    global_registry_ = new FlagRegistry;
  }
  return global_registry_;
}

// --------------------------------------------------------------------
// CommandLineFlagParser
//    Parsing is done in two stages.  In the first, we go through
//    argv.  For every flag-like arg we can make sense of, we parse
//    it and set the appropriate FLAGS_* variable.  For every flag-
//    like arg we can't make sense of, we store it in a vector,
//    along with an explanation of the trouble.  In stage 2, we
//    handle the 'reporting' flags like --help and --mpm_version.
//    (This is via a call to HandleCommandLineHelpFlags(), in
//    gflags_reporting.cc.)
//    An optional stage 3 prints out the error messages.
//       This is a bit of a simplification.  For instance, --flagfile
//    is handled as soon as it's seen in stage 1, not in stage 2.
// --------------------------------------------------------------------
/**
 *     命令行解析器
 * 用户定义的 Flag 有了(统统注册到了 FalgRegistry)
 * 用户传入的参数也有了(统统保存到了 static 中)
 *    有了上面这两个, 就可以进行真正的解析了. 将插入的值保留到对应设置的 Flag 当中
 * 
 * 上面说这个解析分两个阶段:
 *  1. 能解析的就直接保存, 不能解析的就转存到一个 vector 中, 并附带一个解析信息
 *  2. 处理类似 Help 等的报告 Flag
 *  3. 输出错误信息(可选的)
 * 
 */
class CommandLineFlagParser {
 public:
  // The argument is the flag-registry to register the parsed flags in
  explicit CommandLineFlagParser(FlagRegistry* reg) : registry_(reg) {}
  ~CommandLineFlagParser() {}

  // Stage 1: Every time this is called, it reads all flags in argv.
  // However, it ignores all flags that have been successfully set
  // before.  Typically this is only called once, so this 'reparsing'
  // behavior isn't important.  It can be useful when trying to
  // reparse after loading a dll, though.
  uint32 ParseNewCommandLineFlags(int* argc, char*** argv, bool remove_flags);

  // Stage 2: print reporting info and exit, if requested.
  // In gflags_reporting.cc:HandleCommandLineHelpFlags().

  // Stage 3: validate all the commandline flags that have validators
  // registered and were not set/modified by ParseNewCommandLineFlags.
  void ValidateFlags(bool all);
  void ValidateUnmodifiedFlags();

  // Stage 4: report any errors and return true if any were found.
  bool ReportErrors();

  // Set a particular command line option.  "newval" is a string
  // describing the new value that the option has been set to.  If
  // option_name does not specify a valid option name, or value is not
  // a valid value for option_name, newval is empty.  Does recursive
  // processing for --flagfile and --fromenv.  Returns the new value
  // if everything went ok, or empty-string if not.  (Actually, the
  // return-string could hold many flag/value pairs due to --flagfile.)
  // NB: Must have called registry_->Lock() before calling this function.
  string ProcessSingleOptionLocked(CommandLineFlag* flag,
                                   const char* value,
                                   FlagSettingMode set_mode);

  // Set a whole batch of command line options as specified by contentdata,
  // which is in flagfile format (and probably has been read from a flagfile).
  // Returns the new value if everything went ok, or empty-string if
  // not.  (Actually, the return-string could hold many flag/value
  // pairs due to --flagfile.)
  // NB: Must have called registry_->Lock() before calling this function.
  string ProcessOptionsFromStringLocked(const string& contentdata,
                                        FlagSettingMode set_mode);

  // These are the 'recursive' flags, defined at the top of this file.
  // Whenever we see these flags on the commandline, we must take action.
  // These are called by ProcessSingleOptionLocked and, similarly, return
  // new values if everything went ok, or the empty-string if not.
  string ProcessFlagfileLocked(const string& flagval, FlagSettingMode set_mode);
  // diff fromenv/tryfromenv
  string ProcessFromenvLocked(const string& flagval, FlagSettingMode set_mode,
                              bool errors_are_fatal);

 private:
  FlagRegistry* const registry_;
  map<string, string> error_flags_;      // map from name to error message
  // This could be a set<string>, but we reuse the map to minimize the .o size
  map<string, string> undefined_names_;  // --[flag] name was not registered
};


// Parse a list of (comma-separated) flags.
/**
 *     --flagfile 中允许用 "," 来分隔不同的相
 *
 * @param [in]  value   --flagfile 传入的值
 * @param [out] flags   --flagfile 中各项的值 
 * @return     无
 */
static void ParseFlagList(const char* value, vector<string>* flags) {
  for (const char *p = value; p && *p; value = p) {
    p = strchr(value, ',');
    size_t len;
    if (p) {
      len = p - value;
      p++;
    } else {
      len = strlen(value);
    }

    if (len == 0)
      ReportError(DIE, "ERROR: empty flaglist entry\n");
    if (value[0] == '-')
      ReportError(DIE, "ERROR: flag \"%*s\" begins with '-'\n", len, value);

    flags->push_back(string(value, len));
  }
}

// Snarf an entire file into a C++ string.  This is just so that we
// can do all the I/O in one place and not worry about it everywhere.
// Plus, it's convenient to have the whole file contents at hand.
// Adds a newline at the end of the file.
#define PFATAL(s)  do { perror(s); gflags_exitfunc(1); } while (0)

/**
 *     读取文件内容到字符串
 */
static string ReadFileIntoString(const char* filename) {
  const int kBufSize = 8092;
  char buffer[kBufSize];
  string s;
  FILE* fp;
  if ((errno = SafeFOpen(&fp, filename, "r")) != 0) PFATAL(filename);
  size_t n;
  while ( (n=fread(buffer, 1, kBufSize, fp)) > 0 ) {
    if (ferror(fp))  PFATAL(filename);
    s.append(buffer, n);
  }
  fclose(fp);
  return s;
}

/**
 *     解析命令行参数, 并将解析到的值传入到注册好的 flag 中进行保存
 *
 * @param [in]  argc            参数个数
 * @param [in]  argv            具体参数
 * @param [in]  remove_flags    是否删除成功解析过的参数
 * @return     返回能够继续被识别的参数个数, 包含程序名
 */
uint32 CommandLineFlagParser::ParseNewCommandLineFlags(int* argc, char*** argv,
                                                       bool remove_flags) {
  int first_nonopt = *argc;        // for non-options moved to the end

  registry_->Lock();
  for (int i = 1; i < first_nonopt; i++) {  // 不考虑程序名, 仅考虑参数
    char* arg = (*argv)[i];

    /* 将没有选项标记 -/-- 的选项, 一个一个的移动到参数的后面 */
    // Like getopt(), we permute non-option flags to be at the end.
    if (arg[0] != '-' || arg[1] == '\0') {	// must be a program argument: "-" is an argument, not a flag
      /* 由src所指内存区域复制count个字节到dest所指内存区域。 */
      /* 调整了二维数组中的元素顺序, 将当前到最后的元素统一向前移动一个位置 */
      memmove((*argv) + i, (*argv) + i+1, (*argc - (i+1)) * sizeof((*argv)[i]));
      (*argv)[*argc-1] = arg;      // we go last  当前的参数放在最后
      first_nonopt--;              // we've been pushed onto the stack
      i--;                         // to undo the i++ in the loop
      continue;
    }
    arg++;                     // skip leading '-'  原先 arg 是 -, 再 ++ 一次就到了 -- 中的第二个
    if (arg[0] == '-') arg++;  // or leading '--'   也就是这里, 这里就可以很好的去掉 - 和 -- 的问题
                                // 如果我做, 我会用掉太多的函数库, 并不高效, 这种一点一点字节的处理
                                // 应该好好学习

    // -- alone means what it does for GNU: stop options parsing
    // o 单独 - / -- 意味着结束?
    if (*arg == '\0') {
      first_nonopt = i+1;
      break;
    }

    // Find the flag object for this option
    string key;           /* 保存匹配上的 key 值, 没匹配到就保存 */
    const char* value;    /* 保存取得的命令行参数值 */
    string error_message;

    /* arg 就是传入的一个命令行参数 */
    CommandLineFlag* flag = registry_->SplitArgumentLocked(arg, &key, &value,
                                                           &error_message);
    if (flag == NULL) {   /* 传入的标记是没有注册过的, 就跳过 */
      undefined_names_[key] = "";    // value isn't actually used
      error_flags_[key] = error_message;
      continue;
    }

    if (value == NULL) {  /* value == NULL 说明, 当前 arg 中没有 = 号, 要另做处理 */
      // Boolean options are always assigned a value by SplitArgumentLocked()
      /* 应该不会是 bool 类型, 因为 SplitArgumentLocked 会对 bool 类型赋值
      这里也能看出来, gflags 是支持仅传入 bool 标记而不用值的 */
      assert(flag->Type() != FlagValue::FV_BOOL); 
      if (i+1 >= first_nonopt) {  /* 说明后面已经没有参数了, 这就说明 - 标记的这个东西是为赋值的, 毕竟能在注册表中匹配到, 就说明传递的参数是正确的, 只不过没有赋值而已 */
        // This flag needs a value, but there is nothing available
        error_flags_[key] = (string(kError) + "flag '" + (*argv)[i] + "'"
                             + " is missing its argument");
        if (flag->help() && flag->help()[0] > '\001') {
          // Be useful in case we have a non-stripped description.
          error_flags_[key] += string("; flag description: ") + flag->help();
        }
        error_flags_[key] += "\n";
        break;    // we treat this as an unrecoverable error
      } else {
        /* 没有 = 但是后面还有参数, 就继续向后面读一项 */
        value = (*argv)[++i];                   // read next arg for value

        // Heuristic to detect the case where someone treats a string arg
        // like a bool: 启发式检测某人将字符串参数视为bool类型的情况:
        // --my_string_var --foo=bar
        // We look for a flag of string type, whose value begins with a
        // dash, and where the flag-name and value are separated by a
        // space rather than an '='.
        // To avoid false positives, we also require the word "true"
        // or "false" in the help string.  Without this, a valid usage
        // "-lat -30.5" would trigger the warning.  The common cases we
        // want to solve talk about true and false as values.
        // 特别处理的就是在使用空格进行分隔时, 并且接收一个字符串, 而后面一个是以 - 开头的
        // 要解决的就是后面这个 - 是真正所需要的, 还是是另一个标记的开始
        // 只有当前标记接收的是 string, 才会这个问题.
        // 使用层名帮助解决这个问题的办法是, 传递的时候使用 "" 将字符串包裹起来
        // 一定会有 true 和 false 吗? 不一定吧, 对就因为不一定, 可能有人会将 string
        // 标记像 bool 标记一样使用, 这里就会做一个提醒
        if (value[0] == '-'
            && flag->Type() == FlagValue::FV_STRING
            && (strstr(flag->help(), "true")    
                || strstr(flag->help(), "false"))) {
          LOG(WARNING) << "Did you really mean to set flag '"
                       << flag->name() << "' to the value '"  
                       << value << "'?";
        }
      }
    }

    // TODO(csilvers): only set a flag if we hadn't set it before here
    ProcessSingleOptionLocked(flag, value, SET_FLAGS_VALUE);
  }
  registry_->Unlock();

  /* 这一段的作用, 就是表明是否, 将解析过的 flag 都去掉 */
  if (remove_flags) {   // Fix up argc and argv by removing command line flags
    (*argv)[first_nonopt-1] = (*argv)[0];
    (*argv) += (first_nonopt-1);
    (*argc) -= (first_nonopt-1);
    first_nonopt = 1;   // because we still don't count argv[0]
  }

  logging_is_probably_set_up = true;   // because we've parsed --logdir, etc.
  return first_nonopt;
}

/**
 *     允许
 *
 * @param [in]  flagval   flagfile 传入的参数
 * @param [in]  set_mode  写入模式
 * @return     错误信息
 */
string CommandLineFlagParser::ProcessFlagfileLocked(const string& flagval,
                                                    FlagSettingMode set_mode) {
  if (flagval.empty())
    return "";

  string msg;
  vector<string> filename_list;
  ParseFlagList(flagval.c_str(), &filename_list);  // take a list of filenames
  for (size_t i = 0; i < filename_list.size(); ++i) {
    const char* file = filename_list[i].c_str();
    msg += ProcessOptionsFromStringLocked(ReadFileIntoString(file), set_mode);
  }
  return msg;
}

/**
 *     从环境变量中获取所需的标记
 *
 * @param [in]  flagval   --fromenv 可以使用 "," 来分隔可以从环境变了中获取值
 *                          只需要定义的 flag 名字
 * @param [in]  set_mode  设置 Flag 的模式
 * @param [in]  errors_are_fatal  标记是否没从环境变量中找到就直接返回错误信息
 * @return     错误信息
 */
string CommandLineFlagParser::ProcessFromenvLocked(const string& flagval,
                                                   FlagSettingMode set_mode,
                                                   bool errors_are_fatal) {
  if (flagval.empty())
    return "";

  string msg;
  vector<string> flaglist;
  ParseFlagList(flagval.c_str(), &flaglist);

  for (size_t i = 0; i < flaglist.size(); ++i) {
    const char* flagname = flaglist[i].c_str();
    /* 这里就看出来了是通过名字进行查找的 */
    CommandLineFlag* flag = registry_->FindFlagLocked(flagname);
    if (flag == NULL) {
      error_flags_[flagname] =
          StringPrintf("%sunknown command line flag '%s' "
                       "(via --fromenv or --tryfromenv)\n",
                       kError, flagname);
      undefined_names_[flagname] = "";
      continue;
    }

    /* 这里又是 通过 FLAGS_xxx 的方式从环境变量中取值, 所以
    设置环境变量的时候要用 FLAGS_xxx, 但是 --fromenv 传递的时候只需要名字 */
    const string envname = string("FLAGS_") + string(flagname);
    string envval;
    /* 这里就是从环境变量中获取值 */
    if (!SafeGetEnv(envname.c_str(), envval)) {
      if (errors_are_fatal) {
        error_flags_[flagname] = (string(kError) + envname +
                                  " not found in environment\n");
      }
      continue;
    }

    // Avoid infinite recursion.
    if (envval == "fromenv" || envval == "tryfromenv") {
      error_flags_[flagname] =
          StringPrintf("%sinfinite recursion on environment flag '%s'\n",
                       kError, envval.c_str());
      continue;
    }

    /* 递归了 */
    msg += ProcessSingleOptionLocked(flag, envval.c_str(), set_mode);
  }
  return msg;
}

/**
 *  将参数值按照指定的模式写入到 flag 中
 *  当参数是 flagfile 时, 解析传入的文件, 从文件中继续找
 *  当参数是 fromenv 时, 从环境变量中找指定的 flag, 如果没有就报错, 不会使用默认值
 *  当参数是 tryfromenv 时, 从环境变量中找指定的 flag, 如果没有就用默认值
 *
 * @param [in]  flag      需要写入的 flag
 * @param [in]  value     待写入的值
 * @param [in]  set_mode  写入模式
 * @return     错误信息
 */
string CommandLineFlagParser::ProcessSingleOptionLocked(
    CommandLineFlag* flag, const char* value, FlagSettingMode set_mode) {
  string msg;
  if (value && !registry_->SetFlagLocked(flag, value, set_mode, &msg)) {
    error_flags_[flag->name()] = msg;
    return "";
  }

  /* 可以通过 --flagfile 传入存储 flag 的文件, 在这个文件中只能一行一个标记的存,
  并且可以使用程序名作为不同段的分隔 */
  // The recursive flags, --flagfile and --fromenv and --tryfromenv,
  // must be dealt with as soon as they're seen.  They will emit
  // messages of their own.

  /* flagfile fromenv tryfromenv 是自带的, 所以经过上面的赋值肯定是已经有值了
    所以就可以直接使用了 */
  if (strcmp(flag->name(), "flagfile") == 0) {
    msg += ProcessFlagfileLocked(FLAGS_flagfile, set_mode);

  } else if (strcmp(flag->name(), "fromenv") == 0) {
    // last arg indicates envval-not-found is fatal (unlike in --tryfromenv)
    msg += ProcessFromenvLocked(FLAGS_fromenv, set_mode, true);

    /* 这俩有啥作用吗? 没 GET 到, 后面的 true/false 指出是否没找到时就直接退出 */
  } else if (strcmp(flag->name(), "tryfromenv") == 0) {
    msg += ProcessFromenvLocked(FLAGS_tryfromenv, set_mode, false);
  }

  return msg;
}

/*  */
void CommandLineFlagParser::ValidateFlags(bool all) {
  FlagRegistryLock frl(registry_);
  for (FlagRegistry::FlagConstIterator i = registry_->flags_.begin();
       i != registry_->flags_.end(); ++i) {
    /* 这里会调用用户传入的检查函数 */
    if ((all || !i->second->Modified()) && !i->second->ValidateCurrent()) {
      // only set a message if one isn't already there.  (If there's
      // an error message, our job is done, even if it's not exactly
      // the same error.)
      /* 为什么空了还要专门处理??? 因为能进到这里来, ValidateCurrent 就已经失败了! */
      if (error_flags_[i->second->name()].empty()) {  
        error_flags_[i->second->name()] =
            string(kError) + "--" + i->second->name() +
            " must be set on the commandline";
        if (!i->second->Modified()) {
          error_flags_[i->second->name()] += " (default value fails validation)";
        }
        error_flags_[i->second->name()] += "\n";
      }
    }
  }
}

void CommandLineFlagParser::ValidateUnmodifiedFlags() {
  ValidateFlags(false);
}

bool CommandLineFlagParser::ReportErrors() {
  // error_flags_ indicates errors we saw while parsing.
  // But we ignore undefined-names if ok'ed by --undef_ok
  if (!FLAGS_undefok.empty()) {
    vector<string> flaglist;
    ParseFlagList(FLAGS_undefok.c_str(), &flaglist);
    for (size_t i = 0; i < flaglist.size(); ++i) {
      // We also deal with --no<flag>, in case the flagname was boolean
      const string no_version = string("no") + flaglist[i];
      if (undefined_names_.find(flaglist[i]) != undefined_names_.end()) {
        error_flags_[flaglist[i]] = "";    // clear the error message
      } else if (undefined_names_.find(no_version) != undefined_names_.end()) {
        error_flags_[no_version] = "";
      }
    }
  }
  // Likewise, if they decided to allow reparsing, all undefined-names
  // are ok; we just silently ignore them now, and hope that a future
  // parse will pick them up somehow.
  if (allow_command_line_reparsing) {
    for (map<string, string>::const_iterator it = undefined_names_.begin();
         it != undefined_names_.end();  ++it)
      error_flags_[it->first] = "";      // clear the error message
  }

  bool found_error = false;
  string error_message;
  for (map<string, string>::const_iterator it = error_flags_.begin();
       it != error_flags_.end(); ++it) {
    if (!it->second.empty()) {
      error_message.append(it->second.data(), it->second.size());
      found_error = true;
    }
  }
  if (found_error)
    ReportError(DO_NOT_DIE, "%s", error_message.c_str());
  return found_error;
}

/**
 *     看来真的是可以将参数暂时先保存到文件中, 然后直接传入一个文件就好!!!
 *  强强强啊
 * 
 *  从传入的文件中, 解析 flag
 *    此时文件中的也有一些额外的规则, 每行只能写入四种内容:
 *    (1) 注释 # 开头
 *    (2) 空行
 *    (3) 文件名  可以是全路径, 也可以仅是文件名, 
 *        不是必须得, 在文件名之外的统统都算是当前程序的标记
 *    (4) -/-- 开头的标记 从这就能看出来, 文件中要一行一个标记
 * 
 * 
 * @param [in]  contentdata   --flagfile 传入的一个文件中的内容
 * @param [in]  set_mode      存储模式
 * @return     错误信息
 */
string CommandLineFlagParser::ProcessOptionsFromStringLocked(
    const string& contentdata, FlagSettingMode set_mode) {
  string retval;
  const char* flagfile_contents = contentdata.c_str();
  bool flags_are_relevant = true;   // set to false when filenames don't match
                                    // 当没设置文件名字时, 默认是当前调用程序的 flag
  bool in_filename_section = false;

  const char* line_end = flagfile_contents;
  // We read this file a line at a time.
  for (; line_end; flagfile_contents = line_end + 1) {
    /* 
      isspace 检查传入的字符是否为空白, 如果字符是空白字符，返回非零值，否则返回零
    所以下面的 while 目的就是跳过存在的空白
     */
    while (*flagfile_contents && isspace(*flagfile_contents))
      ++flagfile_contents;

    // Windows uses "\r\n"
    /* strchr 找到子串, 返回指向 str 找到的字符的指针，若未找到该字符则为空指针。 */
    line_end = strchr(flagfile_contents, '\r');
    if (line_end == NULL) /* 没有找到 \r 就说明是 Unix, Unix 以 '\n' 为结尾 */
        line_end = strchr(flagfile_contents, '\n');

    /* 
    到这里的时候, flagfile_contents 已经指向的是有内容的第一个字符
    len 表示的就是一行有多长

    line_end == NULL 就说明文件只有一行, 所以会没有换行!!!
    感觉这个可以直接使用 std::getline() 很方便的实现
     */
    size_t len = line_end ? line_end - flagfile_contents
                          : strlen(flagfile_contents);
    string line(flagfile_contents, len);

    // Each line can be one of four things:
    // 1) A comment line -- we skip it  注释跳过
    // 2) An empty line -- we skip it   空行跳过
    // 3) A list of filenames -- starts a new filenames+flags section 
    // 4) A --flag=value line -- apply if previous filenames match  
    // 果然这个文件中, 是可以写命令的!!!
    // 并且只能一行一个一名
    // 一行是一些文件列表.
    if (line.empty() || line[0] == '#') { // # 作为注释
      // comment or empty line; just ignore

    } else if (line[0] == '-') {    // flag 找到一个 flag 
      in_filename_section = false;  // instead, it was a flag-line 
      if (!flags_are_relevant)      // skip this flag; applies to someone else
        continue;

      /* 最开始的时候是不向关的字段, 也就是说, 要想使文件中字段有效, 必须使用
      程序名作为开头, 并且文件名前后不能有多余的东西 */

      const char* name_and_val = line.c_str() + 1;    // skip the leading -
      if (*name_and_val == '-')
        name_and_val++;                               // skip second - too
      string key;
      const char* value;
      string error_message;

      /* 这个和前面一样了, 同样能用文件写的, 还只能是在程序中定义过的 flag!!! */
      CommandLineFlag* flag = registry_->SplitArgumentLocked(name_and_val,
                                                             &key, &value,
                                                             &error_message);
      // By API, errors parsing flagfile lines are silently ignored.
      if (flag == NULL) {
        // "WARNING: flagname '" + key + "' not found\n"
      } else if (value == NULL) {
        // "WARNING: flagname '" + key + "' missing a value\n"
      } else {
        
        /* 递归了, 标记文件中, 也是可以再传递标记文件的 */
        retval += ProcessSingleOptionLocked(flag, value, set_mode);
      }

    } else {                        // a filename!  为什么不用 - 开头就说是一个文件名??
                                    // 就是规定的, 上面说了, 每个文件只能写四种东西
      if (!in_filename_section) {   // start over: assume filenames don't match
        in_filename_section = true;
        flags_are_relevant = false;
      }

      // Split the line up at spaces into glob-patterns
      const char* space = line.c_str();   // just has to be non-NULL
      for (const char* word = line.c_str(); *space; word = space+1) {
        if (flags_are_relevant)     // we can stop as soon as we match
          break;
        space = strchr(word, ' ');
        if (space == NULL)    // 跳过空白
          space = word + strlen(word);
        const string glob(word, space - word);

        /* 找打一个单词 */
        // We try matching both against the full argv0 and basename(argv0)
        // 哦哦哦 有点儿明白了, 这些好像是, 可以使用程序名对文件中标记进行分段
        if (glob == ProgramInvocationName()       // small optimization 程序名名字
            || glob == ProgramInvocationShortName() // 只留文件名
#if defined(HAVE_FNMATCH_H)
            || fnmatch(glob.c_str(), ProgramInvocationName(),      FNM_PATHNAME) == 0
            || fnmatch(glob.c_str(), ProgramInvocationShortName(), FNM_PATHNAME) == 0
#elif defined(HAVE_SHLWAPI_H)
            || PathMatchSpecA(glob.c_str(), ProgramInvocationName())
            || PathMatchSpecA(glob.c_str(), ProgramInvocationShortName())
#endif
            ) {
          flags_are_relevant = true;
        }
      }
    }
  }
  return retval;
}

// --------------------------------------------------------------------
// GetFromEnv()
// AddFlagValidator()
//    These are helper functions for routines like BoolFromEnv() and
//    RegisterFlagValidator, defined below.  They're defined here so
//    they can live in the unnamed namespace (which makes friendship
//    declarations for these classes possible).
// --------------------------------------------------------------------

template<typename T>
T GetFromEnv(const char *varname, T dflt) {
  std::string valstr;
  if (SafeGetEnv(varname, valstr)) {
    FlagValue ifv(new T, true);
    if (!ifv.ParseFrom(valstr.c_str())) {
      ReportError(DIE, "ERROR: error parsing env variable '%s' with value '%s'\n",
                  varname, valstr.c_str());
    }
    return OTHER_VALUE_AS(ifv, T);
  } else return dflt;
}

/**
 *     添加一个标记检查器
 *  这个方法是  CommandLineFlag  的友元!!! 所以是可以直接通过成员添加的!!!
 * 
 * @param [in]  flag_ptr            定义的 flag 指针
 * @param [in]  validate_fn_proto   检查函数
 * @return     无
 */
bool AddFlagValidator(const void* flag_ptr, ValidateFnProto validate_fn_proto) {
  // We want a lock around this routine, in case two threads try to
  // add a validator (hopefully the same one!) at once.  We could use
  // our own thread, but we need to loook at the registry anyway, so
  // we just steal that one.
  /* 上面的意思应该是说用了注册表的锁来保证线程安全 */
  FlagRegistry* const registry = FlagRegistry::GlobalRegistry();
  FlagRegistryLock frl(registry);
  // First, find the flag whose current-flag storage is 'flag'.
  // This is the CommandLineFlag whose current_->value_buffer_ == flag
  CommandLineFlag* flag = registry->FindFlagViaPtrLocked(flag_ptr);
  if (!flag) {
    LOG(WARNING) << "Ignoring RegisterValidateFunction() for flag pointer "
                 << flag_ptr << ": no flag found at that address";
    return false;
  } else if (validate_fn_proto == flag->validate_function()) {
    return true;    // ok to register the same function over and over again
  } else if (validate_fn_proto != NULL && flag->validate_function() != NULL) {
    LOG(WARNING) << "Ignoring RegisterValidateFunction() for flag '"
                 << flag->name() << "': validate-fn already registered";
    return false;
  } else {
    flag->validate_fn_proto_ = validate_fn_proto;
    return true;
  }
}

}  // end unnamed namespaces


// 注册相关

// Now define the functions that are exported via the .h file

// --------------------------------------------------------------------
// FlagRegisterer
//    This class exists merely to have a global constructor (the
//    kind that runs before main(), that goes an initializes each
//    flag that's been declared.  Note that it's very important we
//    don't have a destructor that deletes flag_, because that would
//    cause us to delete current_storage/defvalue_storage as well,
//    which can cause a crash if anything tries to access the flag
//    values in a global destructor.
// --------------------------------------------------------------------

/* 匿名命名空间, 这就完全对外屏蔽了 RegisterCommandLineFlag 这个方法
也可以考虑使用匿名空间解决变量重命名的问题, 可以可以!!!
上面这个办法不行哦, 试了一下, 匿名空间也是一个空间, 所有匿名空间中的东西都是一个空间里的, 所以也不能存在命名冲突问题! */
namespace {

/* 进行标记的注册 */
void RegisterCommandLineFlag(const char* name,
                             const char* help,
                             const char* filename,
                             FlagValue* current,
                             FlagValue* defvalue) {
  if (help == NULL)
    help = "";
  // Importantly, flag_ will never be deleted, so storage is always good.
  /* 创建一个空间用来存储定义的标记 */
  CommandLineFlag* flag =
      new CommandLineFlag(name, help, filename, current, defvalue);
  /* 将标记注册到注册表中, 这里注册时, 并没有实际值, 仅仅是默认值, 和分配的空间
  一切都是在 static 时指定的 */
  FlagRegistry::GlobalRegistry()->RegisterFlag(flag);  // default registry
}
}

template <typename FlagType>
FlagRegisterer::FlagRegisterer(const char* name,
                               const char* help,
                               const char* filename,
                               FlagType* current_storage,
                               FlagType* defvalue_storage)
{
  FlagValue* const current = new FlagValue(current_storage, false);
  FlagValue* const defvalue = new FlagValue(defvalue_storage, false);
  RegisterCommandLineFlag(name, help, filename, current, defvalue);
}

// Force compiler to generate code for the given template specialization.
#define INSTANTIATE_FLAG_REGISTERER_CTOR(type)                  \
  template GFLAGS_DLL_DECL FlagRegisterer::FlagRegisterer(      \
      const char* name, const char* help, const char* filename, \
      type* current_storage, type* defvalue_storage)

// Do this for all supported flag types.
INSTANTIATE_FLAG_REGISTERER_CTOR(bool);
INSTANTIATE_FLAG_REGISTERER_CTOR(int32);
INSTANTIATE_FLAG_REGISTERER_CTOR(uint32);
INSTANTIATE_FLAG_REGISTERER_CTOR(int64);
INSTANTIATE_FLAG_REGISTERER_CTOR(uint64);
INSTANTIATE_FLAG_REGISTERER_CTOR(double);
INSTANTIATE_FLAG_REGISTERER_CTOR(std::string);

#undef INSTANTIATE_FLAG_REGISTERER_CTOR

// --------------------------------------------------------------------
// GetAllFlags()
//    The main way the FlagRegistry class exposes its data.  This
//    returns, as strings, all the info about all the flags in
//    the main registry, sorted first by filename they are defined
//    in, and then by flagname.
// --------------------------------------------------------------------
/* 先依据文件名排序, 再依据标记名排序 */
struct FilenameFlagnameCmp {
  bool operator()(const CommandLineFlagInfo& a,
                  const CommandLineFlagInfo& b) const {
    int cmp = strcmp(a.filename.c_str(), b.filename.c_str());
    if (cmp == 0)
      cmp = strcmp(a.name.c_str(), b.name.c_str());  // secondary sort key
    return cmp < 0;
  }
};

/**
 *     获取在注册表 FlagRegistry 中注册的所有标记
 * 
 * GetAllFlags 是  FlagRegistry 类的友元, 所以可以直接访问私有成员
 * 
 * @param [out] OUTPUT      输出位置
 */
void GetAllFlags(vector<CommandLineFlagInfo>* OUTPUT) {
  FlagRegistry* const registry = FlagRegistry::GlobalRegistry();
  registry->Lock();
  for (FlagRegistry::FlagConstIterator i = registry->flags_.begin();
       i != registry->flags_.end(); ++i) {
    CommandLineFlagInfo fi;
    i->second->FillCommandLineFlagInfo(&fi);      /* 深拷贝一份 */
    OUTPUT->push_back(fi);
  }
  registry->Unlock();
  // Now sort the flags, first by filename they occur in, then alphabetically
  sort(OUTPUT->begin(), OUTPUT->end(), FilenameFlagnameCmp());
}

// --------------------------------------------------------------------
// SetArgv()
// GetArgvs()
// GetArgv()
// GetArgv0()
// ProgramInvocationName()
// ProgramInvocationShortName()
// SetUsageMessage()
// ProgramUsage()
//    Functions to set and get argv.  Typically the setter is called
//    by ParseCommandLineFlags.  Also can get the ProgramUsage string,
//    set by SetUsageMessage.
// --------------------------------------------------------------------

// These values are not protected by a Mutex because they are normally
// set only once during program startup.
static string argv0("UNKNOWN");  // just the program name 程序名
static string cmdline;           // the entire command-line 保留完整的命令行参数, 就是将原先的 argv 变成单一的 string 进行保存
static string program_usage;      // 这个应该是用户传入的信息
static vector<string> argvs;      // 这个是保留单独的一个个的命令行参数
static uint32 argv_sum = 0;       // 计算了 cmdline 所有字符的 ASCII 码的和

/**
 *  将 argv 传入的参数拷贝成一整份和一个零散的 vector.
 * 就是上面的 static 变量
 *
 * @param [in]  argc    命令行参数个数
 * @param [in]  argv    命令行参数
 * @return     无
 */
void SetArgv(int argc, const char** argv) {
  static bool called_set_argv = false;
  if (called_set_argv) return;      /* 防止重复设置参数 */
  called_set_argv = true;

  assert(argc > 0); // every program has at least a name
  argv0 = argv[0];                  /* 一个参数是程序名 */

  cmdline.clear();                  
  for (int i = 0; i < argc; i++) {
    if (i != 0) cmdline += " ";     /* 丢弃程序名 */
    cmdline += argv[i];             /* 没有空格的都拼起来 */
    argvs.push_back(argv[i]);       
  }

  // Compute a simple sum of all the chars in argv
  argv_sum = 0;
  for (string::const_iterator c = cmdline.begin(); c != cmdline.end(); ++c) {
    argv_sum += *c;
  }
  /* 为啥要计算字符的和??? */
}

const vector<string>& GetArgvs() { return argvs; }
const char* GetArgv()            { return cmdline.c_str(); }
const char* GetArgv0()           { return argv0.c_str(); }
uint32 GetArgvSum()              { return argv_sum; }
const char* ProgramInvocationName() {             // like the GNU libc fn
  return GetArgv0();
}

/**
 *     只留文件名
 * @return     无
 */
const char* ProgramInvocationShortName() {        // like the GNU libc fn
  size_t pos = argv0.rfind('/');
#ifdef OS_WINDOWS
  if (pos == string::npos) pos = argv0.rfind('\\');
#endif
  return (pos == string::npos ? argv0.c_str() : (argv0.c_str() + pos + 1));
}


// 允许用户设置使用信息
void SetUsageMessage(const string& usage) {
  program_usage = usage;
}

/**
 *     获取显示用户设置的使用信息
 * 
 * @return     使用信息
 */
const char* ProgramUsage() {
  if (program_usage.empty()) {
    return "Warning: SetUsageMessage() never called";
  }
  return program_usage.c_str();
}

// --------------------------------------------------------------------
// SetVersionString()
// VersionString()
// --------------------------------------------------------------------

// 允许用户设置版本信息
static string version_string;

void SetVersionString(const string& version) {
  version_string = version;
}

const char* VersionString() {
  return version_string.c_str();
}


// --------------------------------------------------------------------
// GetCommandLineOption()
// GetCommandLineFlagInfo()
// GetCommandLineFlagInfoOrDie()
// SetCommandLineOption()
// SetCommandLineOptionWithMode()
//    The programmatic way to set a flag's value, using a string
//    for its name rather than the variable itself (that is,
//    SetCommandLineOption("foo", x) rather than FLAGS_foo = x).
//    There's also a bit more flexibility here due to the various
//    set-modes, but typically these are used when you only have
//    that flag's name as a string, perhaps at runtime.
//    All of these work on the default, global registry.
//       For GetCommandLineOption, return false if no such flag
//    is known, true otherwise.  We clear "value" if a suitable
//    flag is found.
// --------------------------------------------------------------------


bool GetCommandLineOption(const char* name, string* value) {
  if (NULL == name)
    return false;
  assert(value);

  FlagRegistry* const registry = FlagRegistry::GlobalRegistry();
  FlagRegistryLock frl(registry);
  CommandLineFlag* flag = registry->FindFlagLocked(name);
  if (flag == NULL) {
    return false;
  } else {
    *value = flag->current_value();
    return true;
  }
}

bool GetCommandLineFlagInfo(const char* name, CommandLineFlagInfo* OUTPUT) {
  if (NULL == name) return false;
  FlagRegistry* const registry = FlagRegistry::GlobalRegistry();
  FlagRegistryLock frl(registry);
  CommandLineFlag* flag = registry->FindFlagLocked(name);
  if (flag == NULL) {
    return false;
  } else {
    assert(OUTPUT);
    flag->FillCommandLineFlagInfo(OUTPUT);
    return true;
  }
}

CommandLineFlagInfo GetCommandLineFlagInfoOrDie(const char* name) {
  CommandLineFlagInfo info;
  if (!GetCommandLineFlagInfo(name, &info)) {
    fprintf(stderr, "FATAL ERROR: flag name '%s' doesn't exist\n", name);
    gflags_exitfunc(1);    // almost certainly gflags_exitfunc()
  }
  return info;
}

string SetCommandLineOptionWithMode(const char* name, const char* value,
                                    FlagSettingMode set_mode) {
  string result;
  FlagRegistry* const registry = FlagRegistry::GlobalRegistry();
  FlagRegistryLock frl(registry);
  CommandLineFlag* flag = registry->FindFlagLocked(name);
  if (flag) {
    CommandLineFlagParser parser(registry);
    result = parser.ProcessSingleOptionLocked(flag, value, set_mode);
    if (!result.empty()) {   // in the error case, we've already logged
      // Could consider logging this change
    }
  }
  // The API of this function is that we return empty string on error
  return result;
}

string SetCommandLineOption(const char* name, const char* value) {
  return SetCommandLineOptionWithMode(name, value, SET_FLAGS_VALUE);
}

// --------------------------------------------------------------------
// FlagSaver
// FlagSaverImpl
//    This class stores the states of all flags at construct time,
//    and restores all flags to that state at destruct time.
//    Its major implementation challenge is that it never modifies
//    pointers in the 'main' registry, so global FLAG_* vars always
//    point to the right place.
// --------------------------------------------------------------------

class FlagSaverImpl {
 public:
  // Constructs an empty FlagSaverImpl object.
  explicit FlagSaverImpl(FlagRegistry* main_registry)
      : main_registry_(main_registry) { }
  ~FlagSaverImpl() {
    // reclaim memory from each of our CommandLineFlags
    vector<CommandLineFlag*>::const_iterator it;
    for (it = backup_registry_.begin(); it != backup_registry_.end(); ++it)
      delete *it;
  }

  // Saves the flag states from the flag registry into this object.
  // It's an error to call this more than once.
  // Must be called when the registry mutex is not held.
  void SaveFromRegistry() {
    FlagRegistryLock frl(main_registry_);
    assert(backup_registry_.empty());   // call only once!
    for (FlagRegistry::FlagConstIterator it = main_registry_->flags_.begin();
         it != main_registry_->flags_.end();
         ++it) {
      const CommandLineFlag* main = it->second;
      // Sets up all the const variables in backup correctly
      CommandLineFlag* backup = new CommandLineFlag(
          main->name(), main->help(), main->filename(),
          main->current_->New(), main->defvalue_->New());
      // Sets up all the non-const variables in backup correctly
      backup->CopyFrom(*main);
      backup_registry_.push_back(backup);   // add it to a convenient list
    }
  }

  // Restores the saved flag states into the flag registry.  We
  // assume no flags were added or deleted from the registry since
  // the SaveFromRegistry; if they were, that's trouble!  Must be
  // called when the registry mutex is not held.
  void RestoreToRegistry() {
    FlagRegistryLock frl(main_registry_);
    vector<CommandLineFlag*>::const_iterator it;
    for (it = backup_registry_.begin(); it != backup_registry_.end(); ++it) {
      CommandLineFlag* main = main_registry_->FindFlagLocked((*it)->name());
      if (main != NULL) {       // if NULL, flag got deleted from registry(!)
        main->CopyFrom(**it);
      }
    }
  }

 private:
  FlagRegistry* const main_registry_;
  vector<CommandLineFlag*> backup_registry_;

  FlagSaverImpl(const FlagSaverImpl&);  // no copying!
  void operator=(const FlagSaverImpl&);
};

FlagSaver::FlagSaver()
    : impl_(new FlagSaverImpl(FlagRegistry::GlobalRegistry())) {
  impl_->SaveFromRegistry();
}

FlagSaver::~FlagSaver() {
  impl_->RestoreToRegistry();
  delete impl_;
}


// --------------------------------------------------------------------
// CommandlineFlagsIntoString()
// ReadFlagsFromString()
// AppendFlagsIntoFile()
// ReadFromFlagsFile()
//    These are mostly-deprecated routines that stick the
//    commandline flags into a file/string and read them back
//    out again.  I can see a use for CommandlineFlagsIntoString,
//    for creating a flagfile, but the rest don't seem that useful
//    -- some, I think, are a poor-man's attempt at FlagSaver --
//    and are included only until we can delete them from callers.
//    Note they don't save --flagfile flags (though they do save
//    the result of having called the flagfile, of course).
// --------------------------------------------------------------------

static string TheseCommandlineFlagsIntoString(
    const vector<CommandLineFlagInfo>& flags) {
  vector<CommandLineFlagInfo>::const_iterator i;

  size_t retval_space = 0;
  for (i = flags.begin(); i != flags.end(); ++i) {
    // An (over)estimate of how much space it will take to print this flag
    retval_space += i->name.length() + i->current_value.length() + 5;
  }

  string retval;
  retval.reserve(retval_space);
  for (i = flags.begin(); i != flags.end(); ++i) {
    retval += "--";
    retval += i->name;
    retval += "=";
    retval += i->current_value;
    retval += "\n";
  }
  return retval;
}

string CommandlineFlagsIntoString() {
  vector<CommandLineFlagInfo> sorted_flags;
  GetAllFlags(&sorted_flags);
  return TheseCommandlineFlagsIntoString(sorted_flags);
}

bool ReadFlagsFromString(const string& flagfilecontents,
                         const char* /*prog_name*/,  // TODO(csilvers): nix this
                         bool errors_are_fatal) {
  FlagRegistry* const registry = FlagRegistry::GlobalRegistry();
  FlagSaverImpl saved_states(registry);
  saved_states.SaveFromRegistry();

  CommandLineFlagParser parser(registry);
  registry->Lock();
  parser.ProcessOptionsFromStringLocked(flagfilecontents, SET_FLAGS_VALUE);
  registry->Unlock();
  // Should we handle --help and such when reading flags from a string?  Sure.
  HandleCommandLineHelpFlags();
  if (parser.ReportErrors()) {
    // Error.  Restore all global flags to their previous values.
    if (errors_are_fatal)
      gflags_exitfunc(1);
    saved_states.RestoreToRegistry();
    return false;
  }
  return true;
}

// TODO(csilvers): nix prog_name in favor of ProgramInvocationShortName()
bool AppendFlagsIntoFile(const string& filename, const char *prog_name) {
  FILE *fp;
  if (SafeFOpen(&fp, filename.c_str(), "a") != 0) {
    return false;
  }

  if (prog_name)
    fprintf(fp, "%s\n", prog_name);

  vector<CommandLineFlagInfo> flags;
  GetAllFlags(&flags);
  // But we don't want --flagfile, which leads to weird recursion issues
  vector<CommandLineFlagInfo>::iterator i;
  for (i = flags.begin(); i != flags.end(); ++i) {
    if (strcmp(i->name.c_str(), "flagfile") == 0) {
      flags.erase(i);
      break;
    }
  }
  fprintf(fp, "%s", TheseCommandlineFlagsIntoString(flags).c_str());

  fclose(fp);
  return true;
}

bool ReadFromFlagsFile(const string& filename, const char* prog_name,
                       bool errors_are_fatal) {
  return ReadFlagsFromString(ReadFileIntoString(filename.c_str()),
                             prog_name, errors_are_fatal);
}


// --------------------------------------------------------------------
// BoolFromEnv()
// Int32FromEnv()
// Uint32FromEnv()
// Int64FromEnv()
// Uint64FromEnv()
// DoubleFromEnv()
// StringFromEnv()
//    Reads the value from the environment and returns it.
//    We use an FlagValue to make the parsing easy.
//    Example usage:
//       DEFINE_bool(myflag, BoolFromEnv("MYFLAG_DEFAULT", false), "whatever");
// --------------------------------------------------------------------

bool BoolFromEnv(const char *v, bool dflt) {
  return GetFromEnv(v, dflt);
}
int32 Int32FromEnv(const char *v, int32 dflt) {
  return GetFromEnv(v, dflt);
}
uint32 Uint32FromEnv(const char *v, uint32 dflt) {
  return GetFromEnv(v, dflt);
}
int64 Int64FromEnv(const char *v, int64 dflt)    {
  return GetFromEnv(v, dflt);
}
uint64 Uint64FromEnv(const char *v, uint64 dflt) {
  return GetFromEnv(v, dflt);
}
double DoubleFromEnv(const char *v, double dflt) {
  return GetFromEnv(v, dflt);
}

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4996) // ignore getenv security warning
#endif
const char *StringFromEnv(const char *varname, const char *dflt) {
  const char* const val = getenv(varname);
  return val ? val : dflt;
}
#ifdef _MSC_VER
#  pragma warning(pop)
#endif


// --------------------------------------------------------------------
// RegisterFlagValidator()
//    RegisterFlagValidator() is the function that clients use to
//    'decorate' a flag with a validation function.  Once this is
//    done, every time the flag is set (including when the flag
//    is parsed from argv), the validator-function is called.
//       These functions return true if the validator was added
//    successfully, or false if not: the flag already has a validator,
//    (only one allowed per flag), the 1st arg isn't a flag, etc.
//       This function is not thread-safe.
// --------------------------------------------------------------------

bool RegisterFlagValidator(const bool* flag,
                           bool (*validate_fn)(const char*, bool)) {
  /* reinterpret_cast 各种类型都能搞, 能把待参数的函数指针转成不带参数的!
  nb 啊 */
  return AddFlagValidator(flag, reinterpret_cast<ValidateFnProto>(validate_fn));
}
bool RegisterFlagValidator(const int32* flag,
                           bool (*validate_fn)(const char*, int32)) {
  return AddFlagValidator(flag, reinterpret_cast<ValidateFnProto>(validate_fn));
}
bool RegisterFlagValidator(const uint32* flag,
                           bool (*validate_fn)(const char*, uint32)) {
  return AddFlagValidator(flag, reinterpret_cast<ValidateFnProto>(validate_fn));
}
bool RegisterFlagValidator(const int64* flag,
                           bool (*validate_fn)(const char*, int64)) {
  return AddFlagValidator(flag, reinterpret_cast<ValidateFnProto>(validate_fn));
}
bool RegisterFlagValidator(const uint64* flag,
                           bool (*validate_fn)(const char*, uint64)) {
  return AddFlagValidator(flag, reinterpret_cast<ValidateFnProto>(validate_fn));
}
bool RegisterFlagValidator(const double* flag,
                           bool (*validate_fn)(const char*, double)) {
  return AddFlagValidator(flag, reinterpret_cast<ValidateFnProto>(validate_fn));
}
bool RegisterFlagValidator(const string* flag,
                           bool (*validate_fn)(const char*, const string&)) {
  return AddFlagValidator(flag, reinterpret_cast<ValidateFnProto>(validate_fn));
}


// --------------------------------------------------------------------
// ParseCommandLineFlags()
// ParseCommandLineNonHelpFlags()
// HandleCommandLineHelpFlags()
//    This is the main function called from main(), to actually
//    parse the commandline.  It modifies argc and argv as described
//    at the top of gflags.h.  You can also divide this
//    function into two parts, if you want to do work between
//    the parsing of the flags and the printing of any help output.
// --------------------------------------------------------------------
/**
 *     解析命令行参数, 并调用校验信息
 *
 * @param [in]  argc    命令行参数个数
 * @param [in]  argv    命令行参数
 * @param [in]  remove_flags    解析完成后释放借成功解析的标记删除
 * @param [in]  do_report       是否开启 helpxxx 系列标记
 * @return     无
 */
static uint32 ParseCommandLineFlagsInternal(int* argc, char*** argv,
                                            bool remove_flags, bool do_report) {
  SetArgv(*argc, const_cast<const char**>(*argv));    // save it for later

  /* 在解析参数之前, 所有用户定义的 Flag 都通过 static 的方式注册到了 FalgRegistry 之中 */
  FlagRegistry* const registry = FlagRegistry::GlobalRegistry();
  CommandLineFlagParser parser(registry);

  // When we parse the commandline flags, we'll handle --flagfile,
  // --tryfromenv, etc. as we see them (since flag-evaluation order
  // may be important).  But sometimes apps set FLAGS_tryfromenv/etc.
  // manually before calling ParseCommandLineFlags.  We want to evaluate
  // those too, as if they were the first flags on the commandline.
  registry->Lock();

  /* 都还没解析, 这里怎么会有值呢? 
  ooo 是因为, 程序中可以在 ParseCommandLineFlags 之前就设置的它们的值
  所以在这里要解析一下, 把之前设置的统统也留下!!!
  */
  parser.ProcessFlagfileLocked(FLAGS_flagfile, SET_FLAGS_VALUE);
  // Last arg here indicates whether flag-not-found is a fatal error or not
  parser.ProcessFromenvLocked(FLAGS_fromenv, SET_FLAGS_VALUE, true);
  parser.ProcessFromenvLocked(FLAGS_tryfromenv, SET_FLAGS_VALUE, false);
  registry->Unlock();

  // Now get the flags specified on the commandline
  const int r = parser.ParseNewCommandLineFlags(argc, argv, remove_flags);

  std::cout << "来到这里\n";

  std::cout << do_report << std::endl;
  if (do_report) {
    std::cout << "do_report\n";

    /* 提供 helpxxx 系列执行处理 */
    HandleCommandLineHelpFlags();   // may cause us to exit on --help, etc.
  }

  // See if any of the unset flags fail their validation checks
  // 调用用户定义的检查函数
  parser.ValidateUnmodifiedFlags();

  /* 打印错误信息就不看了 */
  if (parser.ReportErrors())        // may cause us to exit on illegal flags
    gflags_exitfunc(1);
  return r;
}

/**
 *     解析命令行参数
 *
 * @param [in]  argc    命令行参数格式
 * @param [in]  argv    命令行参数
 * @param [in]  remove_flags  暂不清楚
 * @return     暂不清楚
 */
uint32 ParseCommandLineFlags(int* argc, char*** argv, bool remove_flags) {
  std::cout << "sododsodododsoodsods\n";
  return ParseCommandLineFlagsInternal(argc, argv, remove_flags, true);
}

uint32 ParseCommandLineNonHelpFlags(int* argc, char*** argv,
                                    bool remove_flags) {
  return ParseCommandLineFlagsInternal(argc, argv, remove_flags, false);
}

// --------------------------------------------------------------------
// AllowCommandLineReparsing()
// ReparseCommandLineNonHelpFlags()
//    This is most useful for shared libraries.  The idea is if
//    a flag is defined in a shared library that is dlopen'ed
//    sometime after main(), you can ParseCommandLineFlags before
//    the dlopen, then ReparseCommandLineNonHelpFlags() after the
//    dlopen, to get the new flags.  But you have to explicitly
//    Allow() it; otherwise, you get the normal default behavior
//    of unrecognized flags calling a fatal error.
// TODO(csilvers): this isn't used.  Just delete it?
// --------------------------------------------------------------------

void AllowCommandLineReparsing() {
  allow_command_line_reparsing = true;
}

void ReparseCommandLineNonHelpFlags() {
  // We make a copy of argc and argv to pass in
  const vector<string>& argvs = GetArgvs();
  int tmp_argc = static_cast<int>(argvs.size());
  char** tmp_argv = new char* [tmp_argc + 1];
  for (int i = 0; i < tmp_argc; ++i)
    tmp_argv[i] = strdup(argvs[i].c_str());   // TODO(csilvers): don't dup

  ParseCommandLineNonHelpFlags(&tmp_argc, &tmp_argv, false);

  for (int i = 0; i < tmp_argc; ++i)
    free(tmp_argv[i]);
  delete[] tmp_argv;
}

void ShutDownCommandLineFlags() {
  FlagRegistry::DeleteGlobalRegistry();
}


} // namespace GFLAGS_NAMESPACE
