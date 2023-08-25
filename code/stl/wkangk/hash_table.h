/***************************************************************
 * @copyright  Copyright © 2023 wkangk.
 * @file       hash_table.h
 * @author     wkangk <wangkangchn@163.com>
 * @version    v1.0
 * @brief      哈希表
 * @date       2023-08-21 20:20
 **************************************************************/
#ifndef __WKANGK_STL_HASH_TABLE_E_H__ 
#define __WKANGK_STL_HASH_TABLE_E_H__ 
#include <stdint.h>
#include <algorithm>

#include "vector.h"


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

template <typename Key, typename Value, typename HashFcn, typename ExtractKey, typename EqualKey, typename Alloc=alloc>
class hash_table;



template <typename Key, typename Value, typename HashFcn, typename ExtractKey, typename EqualKey, typename Alloc>
class __hashtable_iterator
{
    typedef hash_table<Key, Value, HashFcn, ExtractKey, EqualKey, Alloc> hashtable;
    typedef __hashtable_iterator<Key, Value, HashFcn, ExtractKey, EqualKey, Alloc> iterator;

    typedef __hashtable_node<Value> node;       /* 具体存储数据的节点 */

    typedef forward_iterator_tag iterator_category;
    typedef Value value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef Value& reference;
    typedef Value* pointer;

public:
    __hashtable_iterator() {}
    __hashtable_iterator(node* n, hashtable* tab) : cur_(n), ht_(tab) {}

    reference operator*() const { return cur_->value_; }
    pointer operator->() const { return &(operator*()); }
    iterator& operator++() 
    {
        /* 先在桶内前进, 到头了之后, 前进到下一个桶 */
        const node* old = cur_;
        cur_ = cur_->next_;
        if (!cur_) {    /* 桶内到头了, 换一下个桶 */
            size_type bucket = ht_->bkt_num(old->value_);   /* 先算出当前元素所在的桶号, 而后再向下移动 */

            while (!cur_ && ++bucket < ht_->buckets_.size()) {
                cur_ = ht_->buckets_[bucket];    /* 有元素, 并且还在表范围之内, 就可以拿到!!! */
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

    node* cur_;         /* 指向当前数据节点 */
    hashtable* ht_;     /* 整个 hashtable 结构 */
};


/* 这个类直接超过来了 */
template <typename Key, typename Value, typename HashFcn,
          typename ExtractKey, typename EqualKey, typename Alloc>
struct __hashtable_const_iterator 
{
    typedef hash_table<Key, Value,  HashFcn, ExtractKey, EqualKey, Alloc>
            hashtable;
    typedef __hashtable_iterator<Key, Value,  HashFcn, 
                                ExtractKey, EqualKey, Alloc>
            iterator;
    typedef __hashtable_const_iterator<Key, Value,  HashFcn, 
                                        ExtractKey, EqualKey, Alloc>
            const_iterator;
    typedef __hashtable_node<Value> node;

    typedef forward_iterator_tag iterator_category;
    typedef Value value_type;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;
    typedef const Value& reference; /* 都是不可变得 */
    typedef const Value* pointer;


    __hashtable_const_iterator(const node* n, const hashtable* tab)
        : cur_(n), ht_(tab) {}
    __hashtable_const_iterator() {}
    __hashtable_const_iterator(const iterator& it) : cur_(it.cur_), ht_(it.ht_) {}
    reference operator*() const { return cur_->value_; }
    pointer operator->() const { return &(operator*()); }

    const_iterator& operator++()
    {
        const node* old = cur_;
        cur_ = cur_->next_;
        if (!cur_) {
            size_type bucket = ht_->bkt_num(old->value_);
            while (!cur_ && ++bucket < ht_->buckets_.size()) {
                cur_ = ht_->buckets_[bucket];
            }
        }
        return *this;
    }
    
    const_iterator operator++(int)
    {
        const_iterator tmp = *this;
        ++*this;
        return tmp;
    }

    bool operator==(const const_iterator& it) const 
    { 
        return cur_ == it.cur_; 
    }

    bool operator!=(const const_iterator& it) const 
    { 
        return cur_ != it.cur_; 
    }

    const node* cur_;
    const hashtable* ht_;
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

template <typename Key, typename Value, typename HashFcn, typename ExtractKey, typename EqualKey, typename Alloc>
class hash_table
{
    typedef __hashtable_node<Value> node;
    typedef simple_alloc<node, Alloc> node_allocator;

public:
    typedef Key key_type;
    typedef Value value_type;
    typedef HashFcn hasher;
    typedef EqualKey key_equal;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef value_type*       pointer;
    typedef const value_type* const_pointer;
    typedef value_type&       reference;
    typedef const value_type& const_reference;

    typedef __hashtable_iterator<Key, Value, HashFcn, ExtractKey, EqualKey, Alloc>  iterator;
    typedef __hashtable_const_iterator<Key, Value, HashFcn, 
                                     ExtractKey, EqualKey, Alloc> const_iterator;

    friend struct __hashtable_iterator<Key, Value, HashFcn, ExtractKey, EqualKey, Alloc>;
    friend struct __hashtable_const_iterator<Key, Value, HashFcn, ExtractKey, EqualKey, Alloc>;

    hash_table(size_type n, const hasher& hf, const key_equal& eql) :
        hash_(hf), equals_(eql), get_key_(ExtractKey()), num_elements_(0)        
    {
        initialize_buckets(n);
    }

    ~hash_table() { clear(); }

public:
    size_type size() const { return num_elements_; }
    size_type bucket_count() const { return buckets_.size(); }
    size_type max_size() const { return size_type(-1); }
    bool empty() const { return size() == 0; }

    iterator begin()
    { 
        for (size_type n = 0; n < buckets_.size(); ++n) {
            if (buckets_[n]) {
                return iterator(buckets_[n], this);
            }
        }
        return end();
    }

    iterator end() { return iterator(0, this); }

    const_iterator begin() const
    {
        for (size_type n = 0; n < buckets_.size(); ++n) {
            if (buckets_[n]) {
                return const_iterator(buckets_[n], this);
            }
        }
        return end();
    }

    const_iterator end() const 
    { 
        return const_iterator(0, this); 
    }


    /* 
        插入时, 若存在重复元素, 则不进行插入
     */
    std::pair<iterator, bool> insert_unique(const value_type& value)
    {
        resize(num_elements_ + 1);   /* 当已有元素数目比桶数大时, 就重新调整桶, 使数据更加分散 */
        return insert_unique_noresize(value);
    }

    iterator insert_euqal(const value_type& value)
    {
        resize(num_elements_ + 1);   /* 当已有元素数目比桶数大时, 就重新调整桶, 使数据更加分散 */
        return insert_equal_noresize(value);
    }

    /* 重新调整桶大小 以及 布局 */
    void resize(size_type num_elements_hit)
    {
        const size_type old_n = buckets_.size();
        if (num_elements_hit > old_n) {
            const size_type n = next_size(old_n);
            if (n > old_n) {
                vector<node*, Alloc> tmp(n, nullptr);
                /* 一个桶一个桶的重新散列 */
                for (size_type bucket = 0; bucket < old_n; ++bucket) {
                    node* first = buckets_[bucket];
                    while (first) {
                        /* 重新散列 */
                        size_type new_bucket = bkt_num(first->value_, n);
                        /* 头插 */
                        buckets_[bucket] = first->next_;
                        first->next_ = tmp[new_bucket];
                        tmp[new_bucket] = first;
                        first = buckets_[bucket];
                    }
                }
                buckets_.swap(tmp);
            }
        }
    }

    void clear()
    {
        for (size_type i = 0; i < buckets_.size(); ++i) {
            node* cur = buckets_[i];
            while (cur != nullptr) {
                node* next = cur->next_;
                delete_node(cur);
                cur = next;
            }
            buckets_[i] = nullptr;
        }

        num_elements_ = 0;

        /* 释放了 桶节点, 但是桶列表 vector 是没有释放的, 这样就可以反复使用,
        这也就是为什么 clear 后, 容器容量是不变的!!! */
    }

private:
    /* 允许插入相同键的数据 */
    iterator insert_equal_noresize(const value_type& value)
    {
        const size_type n = bkt_num(value);
        node* first = buckets_[n];

        for (node* cur = first; cur; cur = cur->next_) {
            if (equals_(get_key_(cur->value_), get_key(value))) {       /* 比较 key, 看是不是一样, 一样就说明冲突了, 直接返回 */
                /* 存在重复, 插入原有元素之前 */
                node* tmp = new_node(value);
                tmp->next_ = cur->next_;
                cur->next_ = tmp;
                ++num_elements_;
                return iterator(tmp, this);
            }
        }

        /* 没有重复键值, 才桶顶进行插入 */
        node* tmp = new_node(value);
        tmp->next_ = first;
        buckets_[n] = tmp;
        ++num_elements_;
        return iterator(tmp, this);
    }

    std::pair<iterator, bool> insert_unique_noresize(const value_type& value)
    {
        const size_type n = bkt_num(value);
        node* first = buckets_[n];

        for (node* cur = first; cur; cur = cur->next_) {
            if (equals_(get_key_(cur->value_), get_key_(value))) {       /* 比较 key, 看是不是一样, 一样就说明冲突了, 直接返回 */
                return {iterator(cur, this), false};
            }
        }

        /* 头插 */
        node* tmp = new_node(value);
        tmp->next_ = first;
        buckets_[n] = tmp;
        ++num_elements_;
        return {iterator(tmp, this), true};
    }

    /* 
        分配指定大小的桶数组
     */
    void initialize_buckets(size_type n)
    {
        const size_type n_buckets = next_size(n);
        buckets_.reserve(n_buckets);    /* 直接借助底层容器进行扩张即可 */
        buckets_.insert(buckets_.end(), n_buckets, (node*)(0));
        num_elements_ = 0;
    }

    /* 获取下一个可用空间 */
    size_type next_size(size_type n)
    {
        return __stl_next_prime(n);
    }

    node* new_node(const value_type& value)
    {
        node* n = node_allocator::allocate();
        n->next_ = nullptr;

        construct(&n->value_, value);
        return n;
    }

    void delete_node(node* n)
    {
        wkangk_stl::destroy(&n->value_);    /* 不是很明白为什么会和标准 stl 起冲突 */
        node_allocator::deallocate(n);
    }

    size_type bkt_num(const value_type& value, size_type n) const
    {
        return bkt_num_key(get_key_(value), n);
    }

    size_type bkt_num(const value_type& value) const
    {
        return bkt_num_key(get_key_(value));
    }

    /* 计算哈希值 */
    size_type bkt_num_key(const key_type& key) const
    {
        return bkt_num_key(key, buckets_.size());
    }

    size_type bkt_num_key(const key_type& key, size_type n) const
    {
        return hash_(key) % n;  /* 这里就能看出, 是通过计算 key 的哈希值, 而后对某个数取余而得到索引 */
    }

private:
    hasher hash_;
    key_equal equals_;
    ExtractKey get_key_;

    vector<node*, Alloc> buckets_;       /* 用 vector 作为底层桶容器 */
    size_type num_elements_;
};


__WKANGK_STL_END_NAMESPACE

#endif