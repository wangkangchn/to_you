/***************************************************************
 * @copyright  Copyright © 2023 wkangk.
 * @file       2_namespace.cpp
 * @author     wkangk <wangkangchn@163.com>
 * @version    v1.0
 * @brief      
 * @date       2023-08-06 11:25
 **************************************************************/

// namespace name
// {
//     int a;
// } // namespace name

// namespace name1
// {
//     int a;
// }

// using name::a;
// using name1::a;

#define GFLAGS_DLL_DECL

namespace fLB {
struct CompileAssert {};
typedef CompileAssert expected_sizeof_double_neq_sizeof_bool[
                      (sizeof(double) != sizeof(bool)) ? 1 : -1];
template<typename From> double GFLAGS_DLL_DECL IsBoolFlag(const From& from);
GFLAGS_DLL_DECL bool IsBoolFlag(bool from);
}  // namespace fLB

/* 

    知道为什么可以不用实现就可以检查输入的参数是不是 bool 类型了.

typedef CompileAssert expected_sizeof_double_neq_sizeof_bool[(sizeof(double) != sizeof(bool)) ? 1 : -1];
这一句是为了检查编译器中 bool 和 double 是不是一样的大小, 因为 c/c++ 中不允许定义负数的数组大小
所以当 bool 和 double 大小一样时, 会定义 -1 的数组大小, 编译报错, 过不了



template<typename From> double GFLAGS_DLL_DECL IsBoolFlag(const From& from);
GFLAGS_DLL_DECL bool IsBoolFlag(bool from);


 */


// Here are the actual DEFINEze for DECLARE use.
#define DEFINE_bool(name, val, txt)                                     \
    namespace fLB {                                                       \
        typedef ::fLB::CompileAssert FLAG_##name##_value_is_not_a_bool[     \
                (sizeof(::fLB::IsBoolFlag(val)) != sizeof(double))? 1: -1]; \
    }                                                                     \

DEFINE_bool(haha, false, "test");

bool fun();

#include <iostream>
#include <string>


struct AAA
{
    AAA() { std::cout << " AAA " << std::endl; }
    virtual ~AAA() = default;
    std::string name;
};

AAA aaa;

struct AA
{
    AA() { std::cout << " AA " << std::endl; }
};

static AA a1;


class Base
{
public:
    virtual ~Base() = default;
};

class Child : public Base
{
public:
    int a;
};


int main()
{   
    std::cout << sizeof(fun()) << std::endl;

    int a1 = 123;
    const int* cp_a1 = const_cast<const int*>(&a1);
    int* p_a1 = const_cast<int*>(cp_a1);
    // const int c_a1 = const_cast<const int>(a1);

    Child child;
    child.a = 99999;
    Base* pbase = dynamic_cast<Base*>(&child);
    Child* pchild = dynamic_cast<Child*>(pbase);
    std::cout << pchild->a << std::endl;

    pbase = static_cast<Base*>(&child);
    pchild = static_cast<Child*>(pbase);
    std::cout << pchild->a << std::endl;

    return 0;
}