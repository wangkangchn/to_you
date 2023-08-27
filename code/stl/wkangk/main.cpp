/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 
时间	   	: 2023-05-19 20:54
***************************************************************/
#include <iostream>
#include <string>
#include <functional>
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
#include "rbtree.h"
#include "set.h"
#include "map.h"
#include "multimap.h"
#include "multiset.h"
#include "hash_table.h"
#include "hash_set.h"
#include "hash_map.h"
#include "hash_multiset.h"
#include "hash_multimap.h"


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


    /* -------------------------------------------------------------------------------
     * rbtree
     * ------------------------------------------------------------------------------- */
    std::cout << "\n\n\nrbtree<int>" << std::endl;


    my_rb_tree<int, int, identity<int>, std::less<int>> itree;
    std::cout << itree.size() << std::endl;
    itree.insert_unique(10);
    itree.insert_unique(7);
    itree.insert_unique(8);
    itree.insert_unique(15);
    itree.insert_unique(5);
    itree.insert_unique(6);
    itree.insert_unique(11);
    itree.insert_unique(13);
    itree.insert_unique(12);
    itree.insert_equal(12);
    std::cout << itree.size() << std::endl;
    for (auto it = itree.begin(); it != itree.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";


    /* -------------------------------------------------------------------------------
     * set
     * ------------------------------------------------------------------------------- */
    std::cout << "\n\nset<int>" << std::endl;
    set<int> iset;
    iset.insert(1);
    iset.insert(10);
    iset.insert(5);
    iset.insert(55);
    iset.insert(25);
    iset.insert(2255);
    iset.insert(255);
    std::cout << "iset.size(): " << iset.size() << std::endl;

    for (auto v : iset) {
        std::cout << v << " ";
    }
    std::cout << std::endl;


    /* -------------------------------------------------------------------------------
     * map<int, std::string
     * ------------------------------------------------------------------------------- */
    std::cout << "\n\nmap<int, std::string>" << std::endl;
    map<int, std::string> my_map;
    for (size_t i = 0; i < 10; ++i) {
        my_map.insert(std::make_pair(i, std::to_string(i)));
        my_map.insert(std::make_pair(i, std::to_string(i)));
    }
    std::cout << "my_map.size(): " << my_map.size() << std::endl;

    for (auto& v : my_map) {
        std::cout << v.first << ": " << v.second << std::endl;
    }


    /* -------------------------------------------------------------------------------
     * multimap<int, std::string
     * ------------------------------------------------------------------------------- */
    std::cout << "\n\nmultimap<int, std::string>" << std::endl;
    multimap<int, std::string> my_multimap;
    for (size_t i = 0; i < 10; ++i) {
        my_multimap.insert(std::make_pair(i, std::to_string(i)));
        my_multimap.insert(std::make_pair(i, std::to_string(i)));
    }
    std::cout << "my_multimap.size(): " << my_multimap.size() << std::endl;

    for (auto& v : my_multimap) {
        std::cout << v.first << ": " << v.second << std::endl;
    }

    /* -------------------------------------------------------------------------------
     * multiset<int>
     * ------------------------------------------------------------------------------- */
    std::cout << "\n\nmultiset<int>" << std::endl;
    multiset<int> my_multiset;
    for (size_t i = 0; i < 10; ++i) {
        my_multiset.insert(i);
        my_multiset.insert(i);
    }
    std::cout << "my_multiset.size(): " << my_multiset.size() << std::endl;

    for (auto& v : my_multiset) {
        std::cout << v  << " ";
    }
    std::cout << std::endl;


    /* -------------------------------------------------------------------------------
     * hash_table
     * ------------------------------------------------------------------------------- */
    std::cout << "\n\nhash_table<int, int>" << std::endl;
    hash_table<int, int, std::hash<int>, identity<int>, equal_to<int>, alloc> ihashtable(10, std::hash<int>(), equal_to<int>());
    for (size_t i = 0; i < 10; ++i) {
        ihashtable.insert_unique(i);
    }

    for (auto& v : ihashtable) {
        std::cout << v  << " ";
    }
    std::cout << std::endl;


    /* -------------------------------------------------------------------------------
     * hash_set
     * ------------------------------------------------------------------------------- */
    std::cout << "\n\nhash_set<int, int>" << std::endl;
    hash_set<int> ihashset(10);
    for (size_t i = 0; i < 20; ++i) {
        ihashset.insert(i);
    }

    for (auto& v : ihashset) {
        std::cout << v  << " ";
    }
    std::cout << std::endl;
    std::cout << "size: " << ihashset.size() << std::endl;
    ihashset.clear();

    ihashset.insert(123);
    ihashset.insert(13);
    for (auto& v : ihashset) {
        std::cout << v  << " ";
    }
    std::cout << std::endl;
    std::cout << "size: " << ihashset.size() << std::endl;


    /* -------------------------------------------------------------------------------
     * hash_map
     * ------------------------------------------------------------------------------- */
    std::cout << "\n\nhash_map<int, int>" << std::endl;
    hash_map<int, int> ihashmap(10);
    for (size_t i = 0; i < 20; ++i) {
        ihashmap.insert({i, i});
    }

    for (auto& v : ihashmap) {
        std::cout << v.first << ": " <<  v.second  << "\n";
    }
    std::cout << "size: " << ihashmap.size() << std::endl;
    ihashmap.clear();

    ihashmap.insert({12, 123});
    ihashmap.insert({32, 13});
    for (auto& v : ihashmap) {
        std::cout << v.first << ": " <<  v.second  << "\n";
    }
    std::cout << "size: " << ihashmap.size() << std::endl;

    /* -------------------------------------------------------------------------------
     * hash_multiset
     * ------------------------------------------------------------------------------- */
    std::cout << "\n\nhash_multiset<float>" << std::endl;
    hash_multiset<float> fhashmultiset(10);
    for (float i = 0; i < 20.f; i += 0.5f) {
        fhashmultiset.insert(i);
        fhashmultiset.insert(i);        /* 允许重复插入的 */
    }

    for (auto& v : fhashmultiset) {
        std::cout << v  << " ";
    }

    std::cout << std::endl;
    std::cout << "size: " << fhashmultiset.size() << std::endl;
    
    fhashmultiset.erase(10.f);
    fhashmultiset.erase(10.f);
    std::cout << "删除 10.f 后大小: " << fhashmultiset.size() << std::endl;
    
    fhashmultiset.clear();

    fhashmultiset.insert(123);
    fhashmultiset.insert(13);
    for (auto& v : fhashmultiset) {
        std::cout << v  << " ";
    }
    std::cout << std::endl;
    std::cout << "size: " << fhashmultiset.size() << std::endl;



    /* -------------------------------------------------------------------------------
     * hash_map
     * ------------------------------------------------------------------------------- */
    std::cout << "\n\nhash_multimap<float, std::string>" << std::endl;
    hash_multimap<float, std::string, std::hash<float>, equal_to<float>> fshash_multimap(10);
    for (float i = 0; i < 20; i += 0.8) {
        fshash_multimap.insert({i, std::to_string(i)});
        fshash_multimap.insert({i, std::to_string(i)});
    }

    for (auto& v : fshash_multimap) {
        std::cout << v.first << ": " <<  v.second  << "\n";
    }
    std::cout << "size: " << fshash_multimap.size() << std::endl;
    fshash_multimap.erase(4.f);
    fshash_multimap.erase(10.f);
    std::cout << "删除 10.f 后大小: " << fshash_multimap.size() << std::endl;
    std::cout << "fshash_multimap.count(4.f): " << fshash_multimap.count(4.f) << std::endl;
    fshash_multimap.clear();

    fshash_multimap.insert({12, "wknakg"});
    fshash_multimap.insert({32, ":qiqi"});
    for (auto& v : fshash_multimap) {
        std::cout << v.first << ": " <<  v.second  << "\n";
    }
    std::cout << "size: " << fshash_multimap.size() << std::endl;

    return 0;
};