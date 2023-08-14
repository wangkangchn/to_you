/***************************************************************
 * @copyright  Copyright © 2023 wkangk.
 * @file       rbtree.h
 * @author     wkangk <wangkangchn@163.com>
 * @version    v1.0
 * @brief      红黑树
 * @date       2023-08-14 20:04
 **************************************************************/
#ifndef __WKANGK_STL_RBTREE_H__ 
#define __WKANGK_STL_RBTREE_H__ 
#include "config.h"
#include "iterator.h"
#include "alloc.h"


__WKANGK_STL_BEGIN_NAMESPACE

typedef bool __rb_tree_color_type;  /* enum int 4 字节, bool 1 字节 */
const __rb_tree_color_type __rb_tree_red = false;   
const __rb_tree_color_type __rb_tree_black = true;


/* 桥梁模式, base 仅负责与其它节点建立连接 */
struct __rb_tree_node_base
{
    typedef __rb_tree_color_type color_type;
    typedef __rb_tree_node_base* base_ptr;

    /* 最小节点, 就是最左节点 */
    static base_ptr minimum(base_ptr node)
    {
        while (node && node->left_) {
            node = node->left_;
        }   
        return node;
    }

    /* 最大节点, 就是最右节点 */
    static base_ptr maximum(base_ptr node)
    {
        while (node && node->right_) {
            node = node->right_;
        }   
        return node;
    }

    color_type color_;
    base_ptr parent_;
    base_ptr left_;
    base_ptr right_;
};

template <typename T>
struct __rb_tree_node : public __rb_tree_node_base
{
    typedef __rb_tree_node<T>* link_type;
    T value_field;      /* 数据域 */
};


/* 迭代器也分为了两个部分 */
struct __rb_tree_iterator_base
{
    typedef __rb_tree_node_base::base_ptr base_ptr;
    typedef bidirectional_iterator_tag iterator_category;   /* 双向迭代器, 不支持随机存取 */
    typedef ptrdiff_t difference_type;

    void increment()    /* 递增, 也就是找右子树的最左 */
    {
        if (node_->right_) {    /* 存在右节点 */
            node_ = __rb_tree_node_base::minimum(node_->right_);    /* 找到右子树的最左 */
        } else {    /* 没有右子树, 下一个应该是父节点吧 */
            base_ptr y = node_->parent_;
            /* 当前是右子树 */
            while (node_ == y->right_) {
                node_ = y;      /* 向上找第一个是左子树的父节点 */
                y = y->parent_;
            }

            /* 当前是左子树, 下一节点就是父节点 */
            if (node_->right_ != y) {
                node_ = y;  /* 当 node_ 指向根时, y 指向 header, header 也表示 end(), 可以可以 */
            }
        }
    }

    /* 后退 */
    void decrement()
    {
        if (node_->color_ == __rb_tree_red && node_->parent_->parent_ == node_) {
            node_ = node_->right_;  /* node 是 header 时, 会出现这种情况 */
        } else if (node_->left_) {  /* 如果有左子树, 就找左子树的最右 */
            node_ = __rb_tree_node_base::maximum(node_->left_);
        } else {    /* 没有子节点, 就要看当前是左子树还是右子树了 */
            base_ptr y = node_->parent_;
            while (node_ == y->left_) { /* 向前找第一个小当前节点小的数, 也就是向上找第一个节点是右子树的父节点 */
                node_ = y;
                y = y->right_;
            }

            node_ = y;  /* 右子树的话, 直接就是父节点, 这个和上面的应该是对称的吧, 上面那个感觉也可以直接写 */
        }
    }

private:
    base_ptr node_;                /* 迭代器总得指向一个节点 */
};


template <typename T, class Ref, class Ptr>
struct __rb_tree_iterator : public __rb_tree_iterator_base
{
    typedef T value_type;
    typedef Ref referencr;
    typedef Ptr pointer;
    typedef __rb_tree_iterator<T, T&, T*> iterator;
    typedef __rb_tree_iterator<T, const T&, const T*> const_iterator;
    typedef __rb_tree_iterator<T, Ref, Ptr> self;
    typedef __rb_tree_node<T>*   link_type;

    __rb_tree_iterator() = default;
    __rb_tree_iterator(link_type x) : node_(x) {}
    __rb_tree_iterator(const iterator& it) : node_(it.node_) {}

    referencr operator*()
    {
        return dynamic_cast<link_type>(node_)->value_field;
    }

    pointer operator->()
    {
        return &(operator*());
    }

    self operator++()   /* 前 */
    {
        increment();
        return *this;
    }

    self operator++(int)    /* 后 */
    {   
        auto tmp = *this;
        increment();
        return tmp;
    }

    self operator--()   /* 前 */
    {
        decrement();
        return *this;
    }

    self operator++(int)    /* 后 */
    {   
        auto tmp = *this;
        decrement();
        return tmp;
    }
};



/* -------------------------------------------------------------------------------
 * RB tree 数据结构
 * ------------------------------------------------------------------------------- */
template <typename Key, typename Value, class KeyOfValue, class Compare, class Alloc=alloc>
struct rb_tree
{
private:
    typedef void* void_pointer;
    typedef __rb_tree_node_base* base_ptr;
    typedef __rb_tree_node<Value> rb_tree_node;
    typedef simple_alloc<rb_tree_node, Alloc> rb_tree_node_allocator;   /* 节点分配器, 一次分配一个节点空间 */
    typedef __rb_tree_color_type color_type;

public:
    typedef Key key_type;
    typedef Value value_type;
    typedef value_type* pointer;
    typedef const pointer const_pointer;
    typedef value_type& reference;
    typedef const reference const_reference;
    typedef rb_tree_node* link_type;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;
    typedef __rb_tree_iterator<value_type, reference, pointer> iterator;

public:
    rb_tree(const Compare& comp=Compare()) :
        node_count_(0), key_compare_(comp)
    {
        init();
    }

    ~rb_tree()
    {
        clear();
        put_node(header_);
    }

public:
    Compare key_compare() const { return key_compare_; }
    iterator begin() const { return leftmost(); }
    iterator end() const { return header_; }    /* 对吧, 看就是 header 表示结尾 */
    bool empty() const { return node_count_ == 0; }
    size_type size() const { return node_count_; }
    size_type maxsize() const { return size_type(-1); }     /* -1 补码表示就是无符号整数的最大值 */


private:
    void init()
    {
        header_ = get_node();
        color(header_) = __rb_tree_red;     /* 总感觉这样用很别扭, 哈哈哈, 我还太嫩了 */
        root() = nullptr;
        leftmost() = header_;
        rightmost() = header_;
    }

    /* 分配一个节点空间 */
    link_type get_node()
    {
        return rb_tree_node_allocator::allocate();
    }

    /* 释放一个节点空间 */
    void put_node(link_type node)
    {
        return rb_tree_node_allocator::deallocate(node);
    }

    /* 申请一个节点, 并调用构造进行初始化 */
    link_type create_node(const value_type& v)
    {
        auto node = get_node();
        construct(&node->value_field_, v)
        return node;
    }

    link_type clone_node(link_type node)    /* 原型 */
    {
        auto node_clone = create_node(node->value_field_);
        node_clone->color_ = node->color_;
        node_clone->left_ = nullptr;
        node_clone->right_ = nullptr;
        return node_clone;
    }

    void destroy_node(link_type node)
    {
        destroy(&p->value_field_);  /* 调用析构 */
        put_node(node);             /* 释放空间 */
    }

    link_type& root() const
    {
        return static_cast<link_type&>(header_->parent_);
    }

    link_type& leftmost() const
    {
        return static_cast<link_type&>(header_->left_);
    }

    link_type& rightmost() const
    {
        return static_cast<link_type&>(header_->right_);
    }

    /* 工具函数, 方便获取节点的成员 */
    static link_type& left(link_type node)
    {
        return static_cast<link_type&>(node->left_);
    }

    static link_type& right(link_type node)
    {
        return static_cast<link_type&>(node->right_);
    }

    static link_type& parent(link_type node)
    {
        return static_cast<link_type&>(node->parent_);
    }

    static reference value(link_type node)
    {
        return node->value_field_;
    }

    static const Key& key(link_type node)
    {
        return KeyOfValue()(value(node));       /* 是通过值来求键值!!! soga */
    }

    static color_type& color(link_type node)
    {
        return (color_type&)(node->color_);
    }


    static link_type& left(base_ptr node)
    {
        return static_cast<link_type&>(node->left_);
    }

    static link_type& right(base_ptr node)
    {
        return static_cast<link_type&>(node->right_);
    }

    static link_type& parent(base_ptr node)
    {
        return static_cast<link_type&>(node->parent_);
    }

    static reference value(base_ptr node)
    {
        return node->value_field_;
    }

    static const Key& key(base_ptr node)
    {
        return KeyOfValue()(value(static_cast<link_type>(node)));       /* 是通过值来求键值!!! soga */
    }

    static color_type& color(base_ptr node)
    {
        return (color_type&)(static_cast<link_type>(node)->color_);
    }

    static link_type minimum(link_type node)
    {
        return static_cast<link_type>(__rb_tree_node_base::minimum(node));
    }

    static link_type maximum(link_type node)
    {
        return static_cast<link_type>(__rb_tree_node_base::maximum(node));
    }


private:
    size_type node_count_;  /* 节点数目 */
    link_type header_;      /* 辅助节点 */
    Compare key_compare_;   /* 键比较器 */
};

__WKANGK_STL_END_NAMESPACE

#endif
