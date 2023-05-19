/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 
时间	   	: 2023-05-19 20:54
***************************************************************/
#include <iostream>
#include <typeinfo> 

#include "iterator.h"

__USEING_WKANGK_STL_NAMESPACE

int main()
{
    int* a = nullptr;
    std::cout << typeid(iterator_category(a)).name() << std::endl;
    std::cout << typeid(value_type(a)).name() << std::endl;
    std::cout << typeid(difference_type(a)).name() << std::endl;

    ptrdiff_t d = 0;
    distance(a, a + 5, d);
    std::cout << d << std::endl;
    std::cout << distance(a, a + 25) << std::endl;

    int* b = a;
    advance(a, 23321);
    std::cout << distance(a, b) << std::endl;
    return 0;
};