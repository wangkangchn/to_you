/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: list.h
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 双向循环链表
时间	   	: 2023-07-19 20:34
***************************************************************/
#ifndef __WKANGK_STL_LIST_HPP__ 
#define __WKANGK_STL_LIST_HPP__ 
#include <algorithm>

#include "config.h"
#include "alloc.h"
#include "construct.h"
#include "uninitialized.h"


__WKANGK_STL_BEGIN_NAMESPACE

/* 链表的一个节点 */
template <typename T>
struct __list_node 
{
    typedef __list_node<T>* pointer;

    pointer prev_;  /* 前一节点指针 */
    pointer next_;  /* 后一节点指针 */
    T data_;        /* 具体数据 */
};


/* list 迭代器 */
template <typename T, typename Ref, typename Ptr>
class __list_iterator
{
    /* 最近突然发现了一个问题是, 大部分库都会将类型的定义放在类开头,
    之前还没太在意, 但是最近看《深入探索 C++ 对象模型》的时候, 发现
    这其实是有原因的, 具体原因请参阅《深入探索 C++ 对象模型》 第 91 页 */
    typedef __list_iterator<T, T&, T*>      iterator;
    typedef __list_iterator<T, Ref, Ptr>    self;

    /* 双向链表既可以向前又可以向后, 但不允许随机访问, 所以定义为双向迭代器
    而不是随机迭代器 */
    typedef bidirectional_iterator_tag  iterator_category;   /* 以下五个类型是迭代器必须要定义的 */
    typedef T                           value_type;             
    typedef ptrdiff_t                   difference_type;        
    typedef Ptr                         pointer;                
    typedef Ref                         reference;             

    typedef __list_node<T>*             link_type;
    typedef size_t                      size_type;

public:
    __list_iterator() = default;
    __list_iterator(const iterator& x) :    /* 拷贝构造 */
        node_(x.node_)
    {
    }
    __list_iterator(link_type node) : node_(node) {}

public:
    /* 为何这里直接判断的是节点的指针类型, 而不是判断具体值类型?
    不是很理解 */
    bool operator==(const self& x) const { return node_ == x.node_; }
    bool operator!=(const self& x) const { return node_ != x.node_; }
    reference operator*() const { return node_->data_; }
    
    /* 书里说下面这些是迭代器成员存取运算子的标准做法
    可能就是这么一种习惯吧. 不管怎么样, 反正看起来很 cool, O(∩_∩)O哈哈~ */
    pointer operator->() const { return &(operator*()); }

    /* 前 ++, 是就地运算, 所以返回引用类型 */
    self& operator++()
    {   
        node_ = static_cast<link_type>(node_->next_);
        return *this;
    }

    /* 而后 c++ 是副本运算, 所以返回常量 */
    self operator++(int)
    {   
        self tmp(*this);
        ++(*this);  /* 借助已经实现的方法, 代码重用都是在一点点细节中体现出来的 */
        return tmp; /* 返回的是旧对象, 因为后 ++ 的语义是在当前语句完成后再操作, 所以要返回旧对象 */
    }

    /* -- 同 ++ 操作 */
    self& operator--()
    {
        node_ = static_cast<link_type>(node_->prev_);
        return *this;
    }

    self operator--(int)
    {
        self tmp(*this);
        --(*this);
        return tmp;
    }

public:
    link_type           node_;      /* 迭代器内部有一个指向真实数据的指针 */
};


/* list 具体容器, 容器应该是由具体迭代器产出
因为只有自己才知道自己怎么遍历 */
template <typename T, typename Alloc = alloc>
class list
{
    typedef __list_node<T>  list_node;
    typedef list_node*      link_type;
    /* 将字节分配器转为字节分配器 */
    typedef simple_alloc<list_node, Alloc>  list_node_allocator;

public:
    typedef __list_iterator<T, T&, T*> iterator;
    typedef T                   value_type;
    typedef value_type&         reference;
    typedef size_t  size_type;

    list()
    {
        empty_initialize();
    }

    ~list()
    {
        clear();
        put_node(node_);
    }

public:
    /* STL 迭代器的区间是左闭右开, 所以 node_ 要指向最后的空节点, 这样才可以满足 STL 规则 */
    iterator begin()
    {
        return static_cast<link_type>(node_->next_);
    }

    iterator end()
    {
        return node_;
    }

    bool empty()
    {
        return node_->next_ == node_;
    }

    size_type size()
    {
        size_type result = 0;
        /* 计算头尾指针的距离 */
        std::distance(begin(), end(), result);
        return result;
    }

    reference front()
    {
        return *begin();
    }

    reference back()
    {
        return *(--end());
    }

    iterator insert(iterator pos, const T& x)
    {
        link_type tmp = create_node(x);
        /* STL 默认是插入到位置的前端 */
        tmp->next_ = pos.node_;
        tmp->prev_ = pos.node_->prev_;

        static_cast<link_type>(pos.node_->prev_)->next_ = tmp;
        pos.node_->prev_ = tmp;     /* 插到当前节点的前面 */
        return tmp;
    }

    void push_back(const T& x)
    {
        insert(end(), x);
    }

    void clear()
    {
        link_type cur = static_cast<link_type>(node_->next_);
        while (cur != node_) {
            link_type tmp = cur;
            cur = static_cast<link_type>(cur->next_);
            destroy_node(tmp);
        }
        node_->next_ = node_;
        node_->prev_ = node_;
    }

    


private:
    /**
     *  分配一个节点
     */
    link_type get_node()
    {
        return list_node_allocator::allocate();
    }
    
    /**
     *  释放一个节点
     */
    void put_node(link_type node)
    {
        list_node_allocator::deallocate(node);
    }

    /**
     *     构建一个新节点并赋值
     *
     * @param [in]  x   待赋值
     * @return     新创建的节点
     */
    link_type create_node(const T& x)
    {
        link_type node = get_node();    /* 先申请空间 */
        construct(&(node->data_), x);   /* 再构造 */
        return node;
    }

    void destroy_node(link_type node)
    {
        destroy(&(node->data_));        /* 先析构 */
        put_node(node);                /* 再释放空间 */
    }

    void empty_initialize()
    {
        node_ = get_node();             /* 空节点是尾节点的下一个节点 */
        node_->next_ = node_;
        node_->prev_ = node_;
    }

private:
    /* 书中指出, 只要一个指针就可以实现头尾指针
    为何? 因为链表不想数组, 内存空间必须是紧挨的, 链表的节点是可以随时增删的
    所以用一个节点就可以实现头尾指针, 只要跟着新添加节点跑就好了 */
    link_type   node_;
};

__WKANGK_STL_END_NAMESPACE

#endif