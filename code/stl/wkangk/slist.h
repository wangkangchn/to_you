/***************************************************************
 * @copyright  Copyright © 2023 wkangk.
 * @file       slist.h
 * @author     wkangk <wangkangchn@163.com>
 * @version    v1.0
 * @brief      单向链表
 * @date       2023-08-07 21:56
 **************************************************************/
#ifndef __WKANGK_STL_SLIST_HPP__ 
#define __WKANGK_STL_SLIST_HPP__ 
#include "iterator.h"
#include "alloc.h"

__WKANGK_STL_BEGIN_NAMESPACE

/* STL 中对单向链表迭代器还做了一些设计, 像是桥梁模型.
将迭代器的遍历和链表的存储分开了来.
而 list 是写死了存储, 我感觉 list 也可以用这种放在来设计
 */

/**
 *  节点存储的基类
 */
struct __slist_node_base
{
    __slist_node_base* next_;   /* 仅提供最基本的链接功能 */
};


/* 某一种具体节点, 存储具体的数据 */
template <typename T>
struct __slist_node : public __slist_node_base
{
    T data_;
};


/**
 *     迭代器负责遍历, 容器负责存储, 迭代器要按照特定的存储规则进行遍历
 * 对于链表, 具体的储存是额外的一个类, 容器进行的是对链表的操作. 而迭代器
 * 仅负责遍历.
 *     slist 桥梁模式的特点是, 将原先具体的固定节点, 变为了可更改的.(我猜测这个是
 * 大佬的尝试, 所以没开放出来)
 */
class __slist_iterator_base
{
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef forward_iterator_tag iterator_category;

    __slist_iterator_base(__slist_node_base* x) : 
        node_(x)
    {
    }

public:
    /* 找到下一个节点 */
    void incr() 
    {
        node_ = node_->next_;
    }

    /* 判断的是两个迭代器是否指向同一个节点. 不同迭代器间肯定是不行的 */
    bool operator==(const __slist_iterator_base x) const
    {
        return node_ == x.node_;
    }

    bool operator!=(const __slist_iterator_base x) const
    {
        return node_ != x.node_;
    }

protected:
    __slist_node_base* node_;       /* 迭代器要保留一个指向数据节点 */
};


template <typename T, typename Ref, typename Ptr>
class __slist_iterator : public __slist_iterator_base
{
public:
    typedef __slist_iterator<T, T&, T*> iterator;   /* iterator 和 self 有什么区别吗? */
    typedef __slist_iterator<T, const T&, const T*> const_iterator;   /* iterator 和 self 有什么区别吗? */
    typedef __slist_iterator<T, Ref, Ptr> self;

    /* 和基类定义的 iterator_category difference_type 构成迭代器所需的五类型 */
    typedef T value_type;
    typedef Ptr pointer;
    typedef Ref reference;
    typedef __slist_node<value_type> list_node;     /* node_ 存的是节点的基类, 这里就定一个了一个具体类来让它指! */

    __slist_iterator(list_node* node) :
        __slist_iterator_base(node)         /* 这样可以将 next 指针从数据节点里分离出来, 强强强 */
    {
    }
    
    __slist_iterator() :
        __slist_iterator_base(nullptr)         /* 链表尾指向 nullptr */
    {
    }

    /* 拷贝构造 */
    __slist_iterator(const iterator& it) :
        __slist_iterator_base(it.node_)         /* 链表尾指向 nullptr */
    {
    }

public:
    reference operator*() const
    {
        return static_cast<list_node*>(node_)->data_;   /* 要将父类指针强转为子类指针 */
    }

    pointer operator->() const
    {
        return &(operator*());
    }

    self& operator++()  /* 前++ */
    {
        incr();
        return *this;
    }
    
    self operator++(int)  /* 后++ */
    {
        self tmp = *this;
        incr();
        return *tmp;
    }

    /* 前向迭代器, 不能实现 -- */

};



/* 头插 将新节点插入到 head 中 */
inline __slist_node_base* __slist_make_link(__slist_node_base* head, __slist_node_base* new_node)
{
    new_node->next_ = head->next_;
    head->next_ = new_node;
    return new_node;
}

inline size_t __slist_size(__slist_node_base* node)
{
    size_t size = 0;
    for (; node->next_ != nullptr; node = node->next_) {
        ++size;
    }
    return size;
}


template <typename T, typename Alloc=alloc>
class slist
{
public:
    typedef T   value_type;
    typedef T*  pointer;
    typedef const pointer const_pointer;
    typedef T&  reference;
    typedef const reference const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef __slist_iterator<value_type, reference, pointer> iterator;
    typedef __slist_iterator<value_type, const_reference, const_pointer> const_iterator;

private:
    typedef __slist_node<value_type> list_node;     /* 都是直接绑定的 */
    typedef __slist_node_base list_node_base;
    typedef __slist_iterator_base iterator_base;
    typedef simple_alloc<list_node, Alloc>  list_node_allocator;

public:
    slist() 
    {
        head_.next_ = nullptr; /* nullptr 标识链接的结尾 */
    }

    ~slist()
    {
        clear();
    }

public:
    iterator begin()
    {   
        /* head 不存节点 */
        return iterator( static_cast<list_node*>(head_.next_) );
    }

    iterator end()
    {
        return iterator(nullptr);   /* 以 nullptr 做结束 */
    }

    size_type size() const
    {
        return __slist_size(head_.next_);
    }

    bool empty() const
    {
        return head_.next_ == nullptr;
    }

    reference front()
    {
        return static_cast<list_node*>(head_.next_)->data_;
    }

    void push_front(const value_type& value)
    {
        __slist_make_link(&head_, create_node(value));
    }

    void pop_front()
    {
        list_node* node = static_cast<list_node*>(head_.next_);
        head_.next_ = head_.next_->next_;
        destroy_node(node);
    }

    void clear()
    {
        while (!empty()) {
            pop_front();
        }
    }

private:
    static list_node* create_node(const value_type& val)
    {
        list_node* node = list_node_allocator::allocate();
        construct(&node->data_, val);
        node->next_ = nullptr;
        return node;
    }

    static void destroy_node(list_node* node)
    {
        destroy(&node->data_);
        list_node_allocator::deallocate(node);
    }

private:
    list_node_base head_;   /* 有一个额外的指针保留链表头 */
};

__WKANGK_STL_END_NAMESPACE


#endif	/* !__WKANGK_STL_SLIST_HPP__ */
