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
#include "stack.h"
#include "queue.h"
#include "heap.h"
#include "priority_queue.h"
#include "slist.h"


__USEING_WKANGK_STL_NAMESPACE


template <typename Container>
void show(const Container& container)
{
    
    for (auto& v : container) {
        std::cout << v << " ";
    }
    std::cout << std::endl;
}

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

    mydeque.pop_back();
    mydeque.pop_back();
    mydeque.pop_back();
    mydeque.pop_back();
    mydeque.pop_back();
    for (auto v : mydeque) {
        std::cout << v << " ";
    }
    std::cout << std::endl;

    /* -------------------------------------------------------------------------------
     * stack
     * ------------------------------------------------------------------------------- */
    std::cout << "\n\nstack<deque>\n";
    stack<int> my_stack;
    for (int i = 190; i < 209; ++i) {
        my_stack.push(i);
    } 
    while (!my_stack.empty()) {
        std::cout << my_stack.top() << " ";
        my_stack.pop();
    } 
    std::cout << std::endl;

    std::cout << "\n\nstack<list>\n";
    stack<int, list<int>> my_stack_list;
    for (int i = 190; i < 209; ++i) {
        my_stack_list.push(i);
    } 
    while (!my_stack_list.empty()) {
        std::cout << my_stack_list.top() << " ";
        my_stack_list.pop();
    } 
    std::cout << std::endl;


    /* -------------------------------------------------------------------------------
     * queue
     * ------------------------------------------------------------------------------- */
    std::cout << "\nqueue<deque>\n";
    queue<int> my_queue;
    for (int i = 290; i < 309; ++i) {
        my_queue.push(i);
    } 
    while (!my_queue.empty()) {
        std::cout << my_queue.front() << " ";
        my_queue.pop();
    } 
    std::cout << std::endl;

    std::cout << "\n\nqueue<list>\n";
    queue<int, list<int>> my_queue_list;
    for (int i = 290; i < 309; ++i) {
        my_queue_list.push(i);
    } 
    while (!my_queue_list.empty()) {
        std::cout << my_queue_list.front() << " ";
        my_queue_list.pop();
    } 
    std::cout << std::endl;


    /* -------------------------------------------------------------------------------
     * heap
     * ------------------------------------------------------------------------------- */
    std::cout << "\n\nheap vector<int>\n";

    vector<int> my_heap;   /* 50 是最后加上的 */
    my_heap.push_back(0);
    my_heap.push_back(1);
    my_heap.push_back(2);
    my_heap.push_back(3);
    my_heap.push_back(4);
    my_heap.push_back(8);
    my_heap.push_back(9);
    my_heap.push_back(3);
    my_heap.push_back(5);

    make_heap(my_heap.begin(), my_heap.end());
    for (auto v: my_heap) {
        std::cout << v << " ";
    }
    std::cout << std::endl;
    
    my_heap.push_back(7);
    push_heap(my_heap.begin(), my_heap.end());
    for (auto v: my_heap) {
        std::cout << v << " ";
    }
    std::cout << std::endl;

    pop_heap(my_heap.begin(), my_heap.end());
    std::cout << my_heap.back() << std::endl;
    my_heap.pop_back();

    /* 68 50 65 31 24 32 */
    sort_heap(my_heap.begin(), my_heap.end());
    for (auto v: my_heap) {
        std::cout << v << " ";
    }
    std::cout << std::endl;

    
    /* -------------------------------------------------------------------------------
     * priority_queue
     * ------------------------------------------------------------------------------- */
    std::cout << "\n\n\npriority_queue" << std::endl;

    int ia[9] = {0, 1, 2, 3, 4, 8, 9, 3, 5};
    priority_queue<int> ipq(ia, ia + 9);
    std::cout << ipq.size() << std::endl;

    for (size_t i = 0; i < ipq.size(); ++i) {
        std::cout << ipq.top() << " ";
    }
    std::cout << std::endl;

    while (!ipq.empty()) {
        std::cout << ipq.top() << " ";
        ipq.pop();
    }
    std::cout << std::endl;


    /* -------------------------------------------------------------------------------
     * slist
     * ------------------------------------------------------------------------------- */
    std::cout << "\n\n\nslist<int>" << std::endl;

    slist<double> my_slist;

    for (size_t i = 0; i < 10; ++i) {
        my_slist.push_front(12.f * i);
    }

    std::cout << "my_slist.size() = " << my_slist.size() << std::endl;
    for (auto& v : my_slist) {
        std::cout << v << " ";
    }
    std::cout << std::endl;

    my_slist.pop_front();
    my_slist.pop_front();
    my_slist.pop_front();

    for (auto& v : my_slist) {
        std::cout << v << " ";
    }
    std::cout << std::endl;
    return 0;
};