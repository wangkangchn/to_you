/***************************************************************
 * @copyright  Copyright © 2023 wkangk.
 * @file       set.h
 * @author     wkangk <wangkangchn@163.com>
 * @version    v1.0
 * @brief      集合
 * @date       2023-08-16 20:55
 **************************************************************/
#ifndef __WKANGK_STL_SET_H__
#define __WKANGK_STL_SET_H__
#include "rbtree.h"

__WKANGK_STL_BEGIN_NAMESPACE

template <class Arg, class Result>
struct unary_function {
    typedef Arg argument_type;
    typedef Result result_type;
};

template <class T>
struct identity : public unary_function<T, T> {
  const T& operator()(const T& x) const { return x; }
};

template <typename Key,
            typename Compare=std::less<Key>,
            typename Alloc=alloc>
class set
{
public:
    /* 键就是值, 值就是键, 就像正常插入一个元素一样!!! */
    typedef Key key_type;
    typedef Key value_type;
    typedef Compare key_compare;
    typedef Compare value_compare;
    typedef size_t size_type;


private:
    /* 通过值找键, 直接返回原值 */
    typedef my_rb_tree<key_type, value_type, identity<value_type>, key_compare, Alloc> rep_type;
    rep_type t_;

public:
    /* 所有的东西都是不可修改的, 为何? 因为 set 是基于红黑树实现的, 而其键就是值, 当我们
    对值进行修改时, 就相当于改了键, 就破坏了红黑树的性质, 但是 stl 无从得知, 就会出现后续的
    问题. 所以在迭代的过程中不能修改 set 的值!!! */
    typedef typename rep_type::const_pointer pointer;
    typedef typename rep_type::const_pointer const_pointer;
    typedef typename rep_type::const_reference reference;
    typedef typename rep_type::const_reference const_reference;
    typedef typename rep_type::const_iterator iterator;

    set() : t_(Compare()) {} 
    explicit set(const Compare& comp) : t_(comp) {} 
    template <typename InputIterator>
    set(InputIterator first, InputIterator last) : t_(Compare())
    {   
        /* 因为 set 是唯一的元素, 所以插入的时候只能使用唯一的插入, 而不能使用相等的插入 */
        t_.insert_unique(first, last);
    }

public:
    /* 不用的就不会编译!!! */
    key_compare key_comp() const
    {
        return t_.key_compare();
    }

    value_compare value_comp() const
    {
        return t_.key_compare();
    }

    iterator begin() const
    {
        return t_.begin();
    }

    iterator end() const
    {
        return t_.end();
    }

    bool empty() const
    {
        return t_.empty();
    }

    size_type size() const
    {
        return t_.size();
    }

    size_type max_size() const
    {
        return t_.max_size();
    }

    std::pair<iterator, bool> insert(const value_type& v)
    {
        return t_.insert_unique(v);
    }

    size_type erase(const key_type& k)
    {
        return t_.erase(k);
    }

    void clear()
    {
        t_.clear();
    }
};

__WKANGK_STL_END_NAMESPACE

#endif	/* !__WKANGK_STL_SET_H__ */
