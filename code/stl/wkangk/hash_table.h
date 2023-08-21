/***************************************************************
 * @copyright  Copyright © 2023 wkangk.
 * @file       hash_table.h
 * @author     wkangk <wangkangchn@163.com>
 * @version    v1.0
 * @brief      哈希表
 * @date       2023-08-21 20:20
 **************************************************************/
#ifndef __WKANGK_STL_DEQUE_H__ 
#define __WKANGK_STL_DEQUE_H__ 
#include <stdint.h>

#include "config.h"
#include "alloc.h"


__WKANGK_STL_BEGIN_NAMESPACE


/**
 *     这个是桶内的一个节点
 */
template <typename Value>
struct __hashtable_node
{   
    Value value_;
    __hashtable_node* next_;
};

template <typename Key, typename Value, typename HashFcn, typename ExtractKey, typename EqualKey, typename Alloc>
class __hashtable_iterator
{
    typedef hashtable<Key, Value, HashFcn, ExtractKey, EqualKey, Alloc> hashtable;
    typedef __hashtable_iterator<Key, Value, HashFcn, ExtractKey, EqualKey, Alloc> iterator;

    typedef __hashtable_node<Value> node;       /* 具体存储数据的节点 */

    typedef forward_iterator_tah iterator_category;
    typedef Value value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef Value& reference;
    typedef Value* pointer;

public:
    __hashtable_iterator() {}

    reference operator*() const { return cur_->value_; }
    pointer operator->() const { return &(operator*()); }
    iterator& operator++() 
    {
        /* 先在桶内前进, 到头了之后, 前进到下一个桶 */
        const node* old = cur_;
        cur_ = cur_->next;
        if (!cur_) {    /* 桶内到头了, 换一下个桶 */
            size_type bucket = ht_->bkt_num(old->value_);   /* 先算出当前元素所在的桶号, 而后再向下移动 */

            while (!cur_ && ++bucket < ht_->buckets.size()) {
                cur_ = ht_->buckets[bucket];    /* 有元素, 并且还在表范围之内, 就可以拿到!!! */
            }
        }

        return *this;
    }
    
    iterator operator++(int)   /* 后++ */
    {
        /* 先在桶内前进, 到头了之后, 前进到下一个桶 */
        iterator tmp = *this;
        ++*this;
        return tmp;
    }

    /* 看的是两个迭代器是否指向了同一个节点 */
    bool operator==(const iterator& it) const 
    {
        return cur_ == it.cur_;
    }

    bool operator!=(const iterator& it) const 
    {
        return cur_ != it.cur_;
    }

private:
    node* cur_;         /* 指向当前数据节点 */
    hashtable* ht_;     /* 整个 hashtable 结构 */
};



static const int __stl_num_primes = 28;     /* 预先找好 28 个整数 */
static const unsigned long __stl_prime_list[__stl_num_primes] = {
    53, 97, 193, 389, 769,
    1543, 3079, 6151, 12289, 24593,
    49157, 98317, 196613, 393241, 786433,
    1572869, 3145739, 6291469, 12582917, 25165843,
    50331653, 100663319, 201326611, 402653189, 805306457,
    1610612741, 3221225473ul, 4294967291ul
};
/* ooo 这些数并不是连续的! 我还以为是连续的来, 还是太嫩了 -.- */

/* 在 28 个质数中, 找出最接近并大于等于 n 的质数 */
inline unsigned long __stl_next_prime(unsigned long n)
{
    const unsigned long* first = __stl_prime_list;
    const unsigned long* last = __stl_prime_list + __stl_num_primes;
    const unsigned long* pos = std::lower_bound(first, last, n);
    return pos == last ? *(last - 1) : *pos;
}

/* 计算总共允许多少个桶 */
size_t max_buncket_count()
{
    return __stl_prime_list[__stl_num_primes - 1];
}
__WKANGK_STL_END_NAMESPACE
