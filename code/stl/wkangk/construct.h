/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: construct.h
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 全局的构造与析构

            这里是创建一个对象的第二步: 调用构造, 初始化成员
        第一步是分配内存空间

时间	   	: 2023-05-24 21:02
***************************************************************/
#ifndef __WAKNGK_STL_CONSTRUCT_HPP__ 
#define __WAKNGK_STL_CONSTRUCT_HPP__ 
#include <new>

#include "config.h"
#include "type_traits.h"
#include "iterator.h"


__WKANGK_STL_BEGIN_NAMESPACE


/* 这个地方只右一个参数吗? 不应该使用可变参数模板??? */
template <typename Class, typename T>
inline void construct(Class* cla, const T& value)
{   
    // 这里是调用 Class::Class(value)
    new (cla) Class(value);   /* 定位new */
}

/* 最泛化的析构, 调用对象的析构 */
template <typename T>
inline void destroy(T* pointer)
{
    pointer->~T();
}



/* 针对 POD 类型, 可以使用更简单的析构方式: 什么都不做, 只需释放内存 */
template <typename ForwardIterator>
inline void __destory_aux(ForwardIterator first, ForwardIterator last, __true_type)
{
}

template <typename ForwardIterator>
inline void __destory_aux(ForwardIterator first, ForwardIterator last, __false_type)
{
    typedef typename iterator_traits<ForwardIterator>::value_type value_type;
    for (auto it = first; it != last; ++it) {
        destroy(&(*it));    /* 调用全局的析构 */
    }
}

template <typename ForwardIterator, typename T>
inline void __destory(ForwardIterator first, ForwardIterator last, T*)
{   
    typedef typename __type_traits<T>::has_trivial_destructor has_trivial_destructor;
    __destory_aux(first, last, has_trivial_destructor());
}

/* 析构一个范围内的对象 */
template <typename ForwardIterator>
inline void destroy(ForwardIterator first, ForwardIterator last)
{
    __destory(first, last, value_type(last));
}

/* 为什么要有这两个特化??? */
inline void destroy(char*, char*) {}
inline void destroy(wchar_t*, wchar_t*) {}

__WKANGK_STL_END_NAMESPACE

#endif	/* !__WAKNGK_STL_CONSTRUCT_HPP__ */
