/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: uninitialized.h
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 对未初始化的内存进行操作
时间	   	: 2023-05-29 21:43
***************************************************************/
#ifndef __WKANGK_STL_UNINITIALIZED_H__ 
#define __WKANGK_STL_UNINITIALIZED_H__ 
#include <string.h>

#include "config.h"
#include "construct.h"
#include "iterator.h"
#include "type_traits.h"


__WKANGK_STL_BEGIN_NAMESPACE


/* -------------------------------------------------------------------------------
 * uninitialized_fill_n
 * ------------------------------------------------------------------------------- */
/* POD 类型 */
template <typename ForwardIterator, typename Size, typename T>
ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& value, __true_type)
{
    return std::fill_n(first, n, value);
}


/* 非 POD 类型 */
template <typename ForwardIterator, typename Size, typename T>
ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& value, __false_type)
{
    ForwardIterator cur = first;
    for (; n > 0; --n, ++cur) {
        /* *cur 是获得接待器数据, & 获得数据地址 */
        construct(&(*cur), value);
    }
    return cur;
}

/* T1 迭代器的值 */
template <typename ForwardIterator, typename Size, typename T, typename T1>
ForwardIterator __uninitialized_fill_n(ForwardIterator first, Size n, const T& value, T1*)
{   
    typedef typename __type_traits<T1>::is_POD_type is_POD;
    return __uninitialized_fill_n_aux(first, n, value, is_POD());
}

/**
 *     使用 T, 从 first 开始填充 n 个值
 * 返回开始位置的迭代器
 */
template <typename ForwardIterator, typename Size, typename T>
ForwardIterator uninitialized_fill_n(ForwardIterator first, Size n, const T& value)
{
    return __uninitialized_fill_n(first, n, value, value_type(first));
}



/* -------------------------------------------------------------------------------
 * uninitialized_copy
 * ------------------------------------------------------------------------------- */
/* POD 类型 */
template <typename InputIterator, typename ForwardIterator>
ForwardIterator __uninitialized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result, __true_type)
{
    return std::copy(first, last, result);
}


/* 非 POD 类型 */
template <typename InputIterator, typename ForwardIterator>
ForwardIterator __uninitialized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result, __false_type)
{
    ForwardIterator cur = result;
    for (; first != last; ++cur, ++first) {
        /* *cur 是获得接待器数据, & 获得数据地址 */
        construct(&(*cur), *first);
    }
    return cur;
}

/* T1 迭代器的值 */
template <typename InputIterator, typename ForwardIterator, typename T1>
ForwardIterator __uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result, T1*)
{   
    typedef typename __type_traits<T1>::is_POD_type is_POD;
    return __uninitialized_copy_aux(first, last, result, is_POD());
}

/**
 *  将 [start, last) 复制到 result
 * 返回结束元素的下一个位置
 */
template <typename InputIterator, typename ForwardIterator>
ForwardIterator uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result)
{
    return __uninitialized_copy(first, last, result, value_type(result));
}


/* 针对指针进行特化 */
char* uninitialized_copy(const char* first, const char* last, char* result)
{
    memmove(result, first, last - first);
    return result + (last - first);
}

wchar_t* uninitialized_copy(const wchar_t* first, const wchar_t* last, wchar_t* result)
{
    memmove(result, first, (last - first) * sizeof(wchar_t));
    return result + (last - first);
}


/* -------------------------------------------------------------------------------
 * uninitialized_fill
 * ------------------------------------------------------------------------------- */
/* POD 类型 */
template <typename ForwardIterator, typename Size, typename T>
void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& value, __true_type)
{
    std::fill(first, last, value);
}


/* 非 POD 类型 */
template <typename ForwardIterator, typename Size, typename T>
void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& value, __false_type)
{
    ForwardIterator cur = first;
    for (; cur != last; ++cur) {
        /* *cur 是获得接待器数据, & 获得数据地址 */
        construct(&(*cur), value);
    }
}

/* T1 迭代器的值 */
template <typename ForwardIterator, typename T, typename T1>
void __uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& value, T1*)
{   
    typedef typename __type_traits<T1>::is_POD_type is_POD;
    return __uninitialized_fill_aux(first, last, value, is_POD());
}

/**
 *     使用 T, 从 first 开始填充 n 个值
 */
template <typename ForwardIterator, typename T>
void uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& value)
{
    return __uninitialized_fill(first, last, value_type(first));
}



__WKANGK_STL_END_NAMESPACE
#endif	/* !__WKANGK_STL_UNINITIALIZED_H__ */

