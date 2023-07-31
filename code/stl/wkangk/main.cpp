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
#include <chrono> 
#include <thread> 

#include "iterator.h"
#include "type_traits.h"
#include "alloc.h"
#include "vector.h"
#include "list.h"
#include "deque.h"


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

    std::cout << typeid(__type_traits<int>::is_POD_type).name() << std::endl;
    std::cout << typeid(__type_traits<int*>::is_POD_type).name() << std::endl;
    std::cout << typeid(__type_traits<const int*>::is_POD_type).name() << std::endl;

    single_client_alloc my_alloc;

    // for (size_t i = 0; i < 1000000000; ++i) {

    //     size_t bytes = 1 << 30;
    //     void* p = my_alloc.allocate(bytes);
    //     std::this_thread::sleep_for(std::chrono::milliseconds(10000000));
    //     my_alloc.deallocate(p, bytes);
    // }

    std::cout << "vector\n";
    vector<int> vec;
    for (int i = 0; i < 9; ++i) {
        vec.push_back(i);
    }

    for (auto v : vec) {
        std::cout << v << " ";
    }
    std::cout << std::endl;


    std::cout << "list\n";
    list<int> my_list;
    for (int i = 9; i < 19; ++i) {
        my_list.push_back(i);
    }

    for (auto v : my_list) {
        std::cout << v << " ";
    }
    std::cout << std::endl;


    std::cout << (int)(unsigned char)(-1) << std::endl;

    std::cout << "\n\ndeque\n";
    deque<int> mydeque;
    for (int i = 19; i < 39; ++i) {
        mydeque.push_back(i);
    }
    for (auto v : mydeque) {
        std::cout << v << " ";
    }
    std::cout << std::endl;

    return 0;
};