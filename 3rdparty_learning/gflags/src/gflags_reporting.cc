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
//
// Revamped and reorganized by Craig Silverstein
//
// This file contains code for handling the 'reporting' flags.  These
// are flags that, when present, cause the program to report some
// information and then exit.  --help and --version are the canonical
// reporting flags, but we also have flags like --helpxml, etc.
//
// There's only one function that's meant to be called externally:
// HandleCommandLineHelpFlags().  (Well, actually, ShowUsageWithFlags(),
// ShowUsageWithFlagsRestrict(), and DescribeOneFlag() can be called
// externally too, but there's little need for it.)  These are all
// declared in the main gflags.h header file.
//
// HandleCommandLineHelpFlags() will check what 'reporting' flags have
// been defined, if any -- the "help" part of the function name is a
// bit misleading -- and do the relevant reporting.  It should be
// called after all flag-values have been assigned, that is, after
// parsing the command-line.
/* 
  这一个文件就是为了打印帮助信息的, soga
 */
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cassert>
#include <string>
#include <vector>

#include "config.h"
#include "gflags/gflags.h"
#include "gflags/gflags_completions.h"
#include "util.h"


// The 'reporting' flags.  They all call gflags_exitfunc().
DEFINE_bool  (help,        false, "show help on all flags [tip: all flags can have two dashes]");
DEFINE_bool  (helpfull,    false, "show help on all flags -- same as -help");
DEFINE_bool  (helpshort,   false, "show help on only the main module for this program");
DEFINE_string(helpon,      "",    "show help on the modules named by this flag value");
DEFINE_string(helpmatch,   "",    "show help on modules whose name contains the specified substr");
DEFINE_bool  (helppackage, false, "show help on all modules in the main package");
DEFINE_bool  (helpxml,     false, "produce an xml version of help");
DEFINE_bool  (version,     false, "show version and build info and exit");


namespace GFLAGS_NAMESPACE {


using std::string;
using std::vector;


// --------------------------------------------------------------------
// DescribeOneFlag()
// DescribeOneFlagInXML()
//    Routines that pretty-print info about a flag.  These use
//    a CommandLineFlagInfo, which is the way the gflags
//    API exposes static info about a flag.
// --------------------------------------------------------------------

static const int kLineLength = 80;    /* 输出每行的最大长度 */

/**
 *     当前行位置还够就空一格, 不够就进行换行
 *  
 * @param [in]  s                   待添加的字符串
 * @param [in out]  final_string    最终的存储结果
 * @param [in out]  chars_in_line   当前行的字符数
 * @return     无
 */
static void AddString(const string& s,
                      string* final_string, int* chars_in_line) {
  const int slen = static_cast<int>(s.length());
  if (*chars_in_line + 1 + slen >= kLineLength) {  // < 80 chars/line
    *final_string += "\n      ";
    *chars_in_line = 6;
  } else {
    *final_string += " ";
    *chars_in_line += 1;
  }
  *final_string += s;
  *chars_in_line += slen;
}

/**
 *     格式化数值字符串
 *
 * @param [in]  flag    命令行标记
 * @param [in]  text    前缀
 * @param [in]  current 表示是否要显示当前值
 * @return     格式化后的字符串
 */
static string PrintStringFlagsWithQuotes(const CommandLineFlagInfo& flag,
                                         const string& text, bool current) {
  /* 看也没法对字符串表示的类型有一个好的区分, 只能使用 if else 进行判别
  c++ 好像真的没有技巧能快速的生成类型哇 */
  const char* c_string = (current ? flag.current_value.c_str() :
                          flag.default_value.c_str());
  if (strcmp(flag.type.c_str(), "string") == 0) {  // add quotes for strings
    return StringPrintf("%s: \"%s\"", text.c_str(), c_string);
  } else {
    return StringPrintf("%s: %s", text.c_str(), c_string);
  }
}

// Create a descriptive string for a flag.
// Goes to some trouble to make pretty line breaks. 花了不少功夫来做漂亮的换行符。
/**
 *     这个才是具体的显示一个 flag 的内容
 *  并且会根据每行的最大字符数进行换行!!!
 *  -名字 (描述) type: xxx default: xxx current: xxx
 * 
 * @param [in]  flag    要显示的 flag 的内容
 * @return     字符串描述
 */
string DescribeOneFlag(const CommandLineFlagInfo& flag) {
  string main_part;
  SStringPrintf(&main_part, "    -%s (%s)",
                flag.name.c_str(),
                flag.description.c_str());
  
  const char* c_string = main_part.c_str();
  int chars_left = static_cast<int>(main_part.length());  /* 标识还有多少字符没有处理 */
  string final_string;    /* 最终输出的字符串, 会将原始输出进行一些换行处理 */
  int chars_in_line = 0;  // how many chars in current line so far? 在最终输出行中, 当前行有多少字符
  
  /* 处理名字和描述 */
  while (1) {
    assert(static_cast<size_t>(chars_left)
            == strlen(c_string));  // Unless there's a \0 in there?
    /* 在一个串中查找给定字符的第一个匹配之处, 没找到返回 NULL */
    const char* newline = strchr(c_string, '\n');
    if (newline == NULL && chars_in_line+chars_left < kLineLength) {
      // The whole remainder of the string fits on this line
      final_string += c_string;
      chars_in_line += chars_left;
      break;
    }
    
    if (newline != NULL && newline - c_string < kLineLength - chars_in_line) {
      int n = static_cast<int>(newline - c_string); /* newline - c_string 当前行中的数目 */
      final_string.append(c_string, n);     /* 没超出行中字符限制, 就直接保存 */
      chars_left -= n + 1;
      c_string += n + 1;
    } else {
      // Find the last whitespace on this 80-char line
      // 超过了一行中字符限制, 就找最后一个空白处, 进行换行
      int whitespace = kLineLength-chars_in_line-1;  // < 80 chars/line 剩下的空间
      while ( whitespace > 0 && !isspace(c_string[whitespace]) ) {
        --whitespace;
      }
      if (whitespace <= 0) {  // 进到这里表示, 当前行剩余的部分中没有空格, 就说明这还是一个单词
        // Couldn't find any whitespace to make a line break.  Just dump the
        // rest out!
        final_string += c_string;   /* 直接存? 就不换行了 */
        chars_in_line = kLineLength;  // next part gets its own line for sure!
        break;
      }
      /* 找到了空白, 就在这个空白处加一个换行 */
      final_string += string(c_string, whitespace);
      chars_in_line += whitespace;
      while (isspace(c_string[whitespace]))  ++whitespace;
      c_string += whitespace;   
      chars_left -= whitespace;
    }
    if (*c_string == '\0')
      break;
    StringAppendF(&final_string, "\n      ");   /* 换行是在这里加的!!! */
    chars_in_line = 6;  /* 空六个字符 */
  }

  // Append data type 处理类型
  AddString(string("type: ") + flag.type, &final_string, &chars_in_line);

  /* 下面就是加上具体默认值和实际值了 */
  // The listed default value will be the actual default from the flag
  // definition in the originating source file, unless the value has
  // subsequently been modified using SetCommandLineOptionWithMode() with mode
  // SET_FLAGS_DEFAULT, or by setting FLAGS_foo = bar before ParseCommandLineFlags().
  // 列出的默认值是定义时候指定的默认值, 除非在 SetCommandLineOptionWithMode() 中
  // 设置了 SET_FLAGS_DEFAULT 来这是新的默认值
  // 或者在 ParseCommandLineFlags() 对值进行了设置
  AddString(PrintStringFlagsWithQuotes(flag, "default", false), &final_string,
            &chars_in_line);
  if (!flag.is_default) { /* 都是默认值的话, 就没必要再显示当前值了
                        这个不一样可能是因为在定义了 flag 之后, 又通过方法
                        进行了修改 */
    AddString(PrintStringFlagsWithQuotes(flag, "currently", true),
              &final_string, &chars_in_line);
  }

  StringAppendF(&final_string, "\n");
  return final_string;
}

// Simple routine to xml-escape a string: escape & and < only.
static string XMLText(const string& txt) {
  string ans = txt;
  for (string::size_type pos = 0; (pos = ans.find("&", pos)) != string::npos; )
    ans.replace(pos++, 1, "&amp;");
  for (string::size_type pos = 0; (pos = ans.find("<", pos)) != string::npos; )
    ans.replace(pos++, 1, "&lt;");
  return ans;
}

static void AddXMLTag(string* r, const char* tag, const string& txt) {
  StringAppendF(r, "<%s>%s</%s>", tag, XMLText(txt).c_str(), tag);
}


static string DescribeOneFlagInXML(const CommandLineFlagInfo& flag) {
  // The file and flagname could have been attributes, but default
  // and meaning need to avoid attribute normalization.  This way it
  // can be parsed by simple programs, in addition to xml parsers.
  string r("<flag>");
  AddXMLTag(&r, "file", flag.filename);
  AddXMLTag(&r, "name", flag.name);
  AddXMLTag(&r, "meaning", flag.description);
  AddXMLTag(&r, "default", flag.default_value);
  AddXMLTag(&r, "current", flag.current_value);
  AddXMLTag(&r, "type", flag.type);
  r += "</flag>";
  return r;
}

// --------------------------------------------------------------------
// ShowUsageWithFlags()
// ShowUsageWithFlagsRestrict()
// ShowXMLOfFlags()
//    These routines variously expose the registry's list of flag
//    values.  ShowUsage*() prints the flag-value information
//    to stdout in a user-readable format (that's what --help uses).
//    The Restrict() version limits what flags are shown.
//    ShowXMLOfFlags() prints the flag-value information to stdout
//    in a machine-readable format.  In all cases, the flags are
//    sorted: first by filename they are defined in, then by flagname.
// --------------------------------------------------------------------

static const char* Basename(const char* filename) {
  const char* sep = strrchr(filename, PATH_SEPARATOR);
  return sep ? sep + 1 : filename;
}

static string Dirname(const string& filename) {
  string::size_type sep = filename.rfind(PATH_SEPARATOR);
  return filename.substr(0, (sep == string::npos) ? 0 : sep);
}

/**
 *  检查文件名中是否有指定的子串 
 * 
 *      我就不是很明白他这个 substrings 到底是干嘛的呀, 很不理解
 *
 * @param [in]  filename    待匹配的文件名
 * @param [in]  substrings  待匹配的子字符串
 * @return     true 匹配到任意一个子串
 */
// Test whether a filename contains at least one of the substrings.
static bool FileMatchesSubstring(const string& filename,
                                 const vector<string>& substrings) {
  for (vector<string>::const_iterator target = substrings.begin();
       target != substrings.end();
       ++target) {
    /* 能否在名字中匹配到一个子串 */
    // std::cout << "filename: " << filename << std::endl;
    // std::cout << "target: " << *target << std::endl;
    if (strstr(filename.c_str(), target->c_str()) != NULL)
      return true;
    // If the substring starts with a '/', that means that we want
    // the string to be at the beginning of a directory component.
    // That should match the first directory component as well, so
    // we allow '/foo' to match a filename of 'foo'.
    /* 如果子字符串以'/'开头，这意味着我们想要位于目录组件开头的字符串。
    它也应该匹配第一个目录组件，所以我们允许'/foo'匹配'foo'的文件名。 */
    /* 也就是说, 当匹配模板是以 / 开头时, 当文件名与模板的中开头匹配时
    也认为匹配成功
    但我还是没理解为啥要这些子串
     */
    if (!target->empty() && (*target)[0] == PATH_SEPARATOR &&
        strncmp(filename.c_str(), target->c_str() + 1,
                strlen(target->c_str() + 1)) == 0)
      return true;
  }
  return false;
}

// Show help for every filename which matches any of the target substrings.
// If substrings is empty, shows help for every file. If a flag's help message
// has been stripped (e.g. by adding '#define STRIP_FLAG_HELP 1'
// before including gflags/gflags.h), then this flag will not be displayed
// by '--help' and its variants.
/**
 *     首先获取用户设置的使用信息
 *  当文件名中没有任何一个要匹配的子串时, 就认为匹配失败
 * 
 * @param [in]  argv0    程序名
 * @param [in]  substrings    待匹配的子串
 * @param [in] 
 * @return     无
 */
static void ShowUsageWithFlagsMatching(const char *argv0,
                                       const vector<string> &substrings) {
  fprintf(stdout, "%s: %s\n", Basename(argv0), ProgramUsage());

  vector<CommandLineFlagInfo> flags;
  GetAllFlags(&flags);           // flags are sorted by filename, then flagname

  string last_filename;          // so we know when we're at a new file
  bool first_directory = true;   // controls blank lines between dirs
  bool found_match = false;      // stays false iff no dir matches restrict
  for (vector<CommandLineFlagInfo>::const_iterator flag = flags.begin();
       flag != flags.end();
       ++flag) {
    if (substrings.empty() ||
        FileMatchesSubstring(flag->filename, substrings)) {
      found_match = true;     // this flag passed the match!
      // If the flag has been stripped, pretend that it doesn't exist.
      if (flag->description == kStrippedFlagHelp) continue;
      if (flag->filename != last_filename) {                      // new file
        if (Dirname(flag->filename) != Dirname(last_filename)) {  // new dir!
          if (!first_directory)
            fprintf(stdout, "\n\n");   // put blank lines between directories
          first_directory = false;
        }
        fprintf(stdout, "\n  Flags from %s:\n", flag->filename.c_str());
        last_filename = flag->filename;
      }
      // Now print this flag
      fprintf(stdout, "%s", DescribeOneFlag(*flag).c_str());
    }
  }

  if (!found_match && !substrings.empty()) {
    fprintf(stdout, "\n  No modules matched: use -help\n");
  }
}

/**
 *     显示在指定格式限制情况下的 flag 信息
 *  
 *  也就是说, 只有当 argv0(程序名) 中, 包含有 restrict_ 指定的格式
 *  才会认为匹配成功, 才会显示相应的 flag
 * 
 * @param [in]  argv0       程序名
 * @param [out] restrict_   制定的匹配格式
 * @return     无
 */
void ShowUsageWithFlagsRestrict(const char *argv0, const char *restrict_) {
  vector<string> substrings;
  if (restrict_ != NULL && *restrict_ != '\0') {
    substrings.push_back(restrict_);
  }
  ShowUsageWithFlagsMatching(argv0, substrings);
}

void ShowUsageWithFlags(const char *argv0) {
  ShowUsageWithFlagsRestrict(argv0, "");
}

// Convert the help, program, and usage to xml.
static void ShowXMLOfFlags(const char *prog_name) {
  vector<CommandLineFlagInfo> flags;
  GetAllFlags(&flags);   // flags are sorted: by filename, then flagname

  // XML.  There is no corresponding schema yet
  fprintf(stdout, "<?xml version=\"1.0\"?>\n");
  // The document
  fprintf(stdout, "<AllFlags>\n");
  // the program name and usage
  fprintf(stdout, "<program>%s</program>\n",
          XMLText(Basename(prog_name)).c_str());
  fprintf(stdout, "<usage>%s</usage>\n",
          XMLText(ProgramUsage()).c_str());
  // All the flags
  for (vector<CommandLineFlagInfo>::const_iterator flag = flags.begin();
       flag != flags.end();
       ++flag) {
    if (flag->description != kStrippedFlagHelp)
      fprintf(stdout, "%s\n", DescribeOneFlagInXML(*flag).c_str());
  }
  // The end of the document
  fprintf(stdout, "</AllFlags>\n");
}

// --------------------------------------------------------------------
// ShowVersion()
//    Called upon --version.  Prints build-related info.
// --------------------------------------------------------------------

static void ShowVersion() {
    /* 用户自己写入的 版本信息 */
  const char* version_string = VersionString();
  if (version_string && *version_string) {
    fprintf(stdout, "%s version %s\n",
            ProgramInvocationShortName(), version_string);
  } else {
    fprintf(stdout, "%s\n", ProgramInvocationShortName());
  }
# if !defined(NDEBUG)
  fprintf(stdout, "Debug build (NDEBUG not #defined)\n");
# endif
}

/**
 *     在程序名字后面加上写额外的后缀
 *  最后变为了 
 *  /名字.
 *  /名字-main.
 *  /名字_main.
 *
 * @param [out] substrings    存储输出结果
 * @param [in]  progname      程序名 
 * @return     无
 */
static void AppendPrognameStrings(vector<string>* substrings,
                                  const char* progname) {
  string r;
  r += PATH_SEPARATOR;
  r += progname;
  substrings->push_back(r + ".");
  substrings->push_back(r + "-main.");
  substrings->push_back(r + "_main.");
}

// --------------------------------------------------------------------
// HandleCommandLineHelpFlags()
//    Checks all the 'reporting' commandline flags to see if any
//    have been set.  If so, handles them appropriately.  Note
//    that all of them, by definition, cause the program to exit
//    if they trigger.
// --------------------------------------------------------------------
/**
 *     用于提供 helpxxx 系列指令的处理
 */
void HandleCommandLineHelpFlags() {
  const char* progname = ProgramInvocationShortName();  // 程序名

  std::cout << "\n\n\n\nHandleCommandLineCompletions\n\n\n";
  HandleCommandLineCompletions();   /* 显示需要的匹配信息 */

  vector<string> substrings;
  AppendPrognameStrings(&substrings, progname);

  /* 这个标记只会打印那些在当前程序文件中记录下定义的 flag
  或者说, 运行程序名, 和 flag 定义的文件名字中科可以进行一定的匹配才会显示
  下面是一个测试结果
    Flags from /home/wkangk/to_you/test/unittest_gflags.cpp:
    -yeah (测试 helpshort) type: bool default: true
   */
  if (FLAGS_helpshort) {  /* 在显式的时候, 对能文件名能匹配上的才进行显示 */
    // show only flags related to this binary:
    // E.g. for fileutil.cc, want flags containing   ... "/fileutil." cc
    std::cout << "\n\nFLAGS_helpshort \n";
    ShowUsageWithFlagsMatching(progname, substrings);
    gflags_exitfunc(1);

  /* 显示所有 flag */
  } else if (FLAGS_help || FLAGS_helpfull) {
    // show all options
    ShowUsageWithFlagsRestrict(progname, "");   // empty restrict
    gflags_exitfunc(1);

  /* 显示在指定模块下的 flag
  helpon 指定要显示的模块名 */
  } else if (!FLAGS_helpon.empty()) {
    string restrict_ = PATH_SEPARATOR + FLAGS_helpon + ".";
    ShowUsageWithFlagsRestrict(progname, restrict_.c_str());
    gflags_exitfunc(1);

  /* 显示包含指定字符串的模块名下的 flag */
  } else if (!FLAGS_helpmatch.empty()) {
    ShowUsageWithFlagsRestrict(progname, FLAGS_helpmatch.c_str());
    gflags_exitfunc(1);

  /* package 的范围是, 和 main() 在同一个目录下的所有文件中的 flag */
  } else if (FLAGS_helppackage) {
    // Shows help for all files in the same directory as main().  We
    // don't want to resort to looking at dirname(progname), because
    // the user can pick progname, and it may not relate to the file
    // where main() resides.  So instead, we search the flags for a
    // filename like "/progname.cc", and take the dirname of that.
    vector<CommandLineFlagInfo> flags;
    GetAllFlags(&flags);
    string last_package;
    for (vector<CommandLineFlagInfo>::const_iterator flag = flags.begin();
         flag != flags.end();
         ++flag) {
      /* 现在只要文件名中有给定的匹配信息就可以 */
      if (!FileMatchesSubstring(flag->filename, substrings))
        continue;

      const string package = Dirname(flag->filename) + PATH_SEPARATOR;
      // 同一个目录下的我们就不考虑多次了, 因为每次找到一个 package 的时候
      // 就会打印出所有 flag
      if (package != last_package) {
        std::cout << "progname: " << progname << ", ";
        std::cout << "package: " << package << std::endl;
        // 后面的 package 找到的就是程序所在的目录了, 后面匹配的时候
        // flag->filename 又是一个全路径, 所以这里匹配的时候就会将
        // 同一目录下的所有 flag 都进行匹配
        ShowUsageWithFlagsRestrict(progname, package.c_str());
        VLOG(7) << "Found package: " << package;
        if (!last_package.empty()) {      // means this isn't our first pkg
          LOG(WARNING) << "Multiple packages contain a file=" << progname;
        }
        last_package = package;
      }
    }
    if (last_package.empty()) {   // never found a package to print
      LOG(WARNING) << "Unable to find a package for file=" << progname;
    }
    gflags_exitfunc(1);

  } else if (FLAGS_helpxml) {
    ShowXMLOfFlags(progname);
    gflags_exitfunc(1);

  } else if (FLAGS_version) {
    ShowVersion();
    // Unlike help, we may be asking for version in a script, so return 0
    gflags_exitfunc(0);

  }
}


} // namespace GFLAGS_NAMESPACE
