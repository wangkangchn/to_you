/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: type_traits.h
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 类型萃取器
时间	   	: 2023-05-23 20:30
***************************************************************/
#include "config.h"

__WKANGK_STL_BEGIN_NAMESPACE

/* 我们希望 __type_traits<T>::xxx 响应我们 "真" 或 "假",
但其结果不应该只是个 bool 值, 应该是个有着真/假性质的"对象",
因为我们希望利用其响应结果进行参数推导, 而编译器只有面对
class object 形式的参数才会做参数推导 */
struct __true_type {};
struct __false_type {};


template <typename T>
struct __type_traits
{
    /* 还不是很理解这个到底是要干嘛 */
    typedef __false_type    this_dummy_member_must_be_first;
    typedef __false_type    has_trivial_default_constructor;
    typedef __false_type    has_trivial_copy_constructor;
    typedef __false_type    has_trivial_assignment_constructor;
    typedef __false_type    has_trivial_destructor;
    typedef __false_type    is_POD_type;

    /* 全部使用 __false_type 是采用的最保守的策略, 使所有类型都可以
    通过. 针对 POD 类型会进行相应的特化 */

    /* 什么是 POD 类型?
    简单的说 POD 类型就是可以直接通过内存拷贝来构造对象. */
};

__STL_TEMPLATE_NULL
struct __type_traits<char>
{
    typedef __true_type     this_dummy_member_must_be_first;
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_constructor;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

__STL_TEMPLATE_NULL
struct __type_traits<unsigned char>
{
    typedef __true_type     this_dummy_member_must_be_first;
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_constructor;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

__STL_TEMPLATE_NULL
struct __type_traits<short>
{
    typedef __true_type     this_dummy_member_must_be_first;
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_constructor;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

__STL_TEMPLATE_NULL
struct __type_traits<unsigned short>
{
    typedef __true_type     this_dummy_member_must_be_first;
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_constructor;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

__STL_TEMPLATE_NULL
struct __type_traits<int>
{
    typedef __true_type     this_dummy_member_must_be_first;
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_constructor;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

__STL_TEMPLATE_NULL
struct __type_traits<unsigned int>
{
    typedef __true_type     this_dummy_member_must_be_first;
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_constructor;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

__STL_TEMPLATE_NULL
struct __type_traits<long>
{
    typedef __true_type     this_dummy_member_must_be_first;
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_constructor;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

__STL_TEMPLATE_NULL
struct __type_traits<unsigned long>
{
    typedef __true_type     this_dummy_member_must_be_first;
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_constructor;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

__STL_TEMPLATE_NULL
struct __type_traits<long long>
{
    typedef __true_type     this_dummy_member_must_be_first;
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_constructor;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

__STL_TEMPLATE_NULL
struct __type_traits<unsigned long long>
{
    typedef __true_type     this_dummy_member_must_be_first;
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_constructor;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

__STL_TEMPLATE_NULL
struct __type_traits<float>
{
    typedef __true_type     this_dummy_member_must_be_first;
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_constructor;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

__STL_TEMPLATE_NULL
struct __type_traits<double>
{
    typedef __true_type     this_dummy_member_must_be_first;
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_constructor;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};

__STL_TEMPLATE_NULL
struct __type_traits<long double>
{
    typedef __true_type     this_dummy_member_must_be_first;
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_constructor;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};


/* 偏特化指针 */
template <typename T>
struct __type_traits<T*>
{
    typedef __true_type     this_dummy_member_must_be_first;
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_constructor;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
};



__WKANGK_STL_END_NAMESPACE