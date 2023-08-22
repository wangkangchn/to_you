/***************************************************************
 * @copyright  Copyright © 2023 wkangk.
 * @file       common.h
 * @author     wkangk <wangkangchn@163.com>
 * @version    v1.0
 * @brief      
 * @date       2023-08-20 20:16
 **************************************************************/
#ifndef __WKANGK_STL_COMMON_H__ 
#define __WKANGK_STL_COMMON_H__ 
#include "config.h"

__WKANGK_STL_BEGIN_NAMESPACE


template <class Arg, class Result>
struct unary_function {
    typedef Arg argument_type;
    typedef Result result_type;
};

template <class T>
struct identity : public unary_function<T, T> 
{
    const T& operator()(const T& x) const { return x; }
};

template <class Pair>
struct select1st : public unary_function<Pair, typename Pair::first_type> {
    const typename Pair::first_type& operator()(const Pair& x) const
    {
        return x.first;
    }
};

template <class Arg1, class Arg2, class Result>
struct binary_function {
    typedef Arg1 first_argument_type;
    typedef Arg2 second_argument_type;
    typedef Result result_type;
}; 

template <class T>
struct equal_to : public binary_function<T, T, bool> {
    bool operator()(const T& x, const T& y) const { return x == y; }
};

__WKANGK_STL_END_NAMESPACE

#endif	/* !__WKANGK_STL_COMMON_H__ */