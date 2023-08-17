/***************************************************************
 * @copyright  Copyright © 2023 wkangk.
 * @file       map.h
 * @author     wkangk <wangkangchn@163.com>
 * @version    v1.0
 * @brief      
 * @date       2023-08-17 20:41
 **************************************************************/
#ifndef __WKANGK_STL_MAP_H__ 
#define __WKANGK_STL_MAP_H__ 
#include "rbtree.h"

__WKANGK_STL_BEGIN_NAMESPACE

template <class Pair>
struct select1st : public unary_function<Pair, typename Pair::first_type> {
    const typename Pair::first_type& operator()(const Pair& x) const
    {
        return x.first;
    }
};

/* 第三个参数是键比较器 */
template <typename Key, typename Value, typename Compare=std::less<Key>, typename Alloc=alloc>
class map
{
public:
    typedef Key key_type;
    typedef Value data_type;    /* 实值 */
    typedef Value mapped_type;
    typedef std::pair<const key_type, data_type> value_type;    /* 整体操作和 set 一致, 只不过一个存 pair, 一个仅存数据 */
    typedef Compare key_compare;

    /* 比较实值, 就是转为键值 */
    class value_compare : public std::binary_function<value_type, value_type, bool>
    {
        friend class map<Key, Value, Compare, Alloc>;   /* 友元的使用, 我还是 get 不到点 */
    
    protected:
        Compare comp;
        value_compare(Compare c) : comp(c) { }

    public:
        bool operator()(const value_type& x, const value_type& y) const 
        {
            return comp(x.first, y.first);
        }
    };

private:
    typedef my_rb_tree<key_type, value_type, select1st<value_type>, key_compare, Alloc> rep_type;
    rep_type t_;

public:
    typedef typename rep_type::pointer pointer;
    typedef typename rep_type::const_pointer const_pointer;
    typedef typename rep_type::reference reference;
    typedef typename rep_type::const_reference const_reference;
    typedef typename rep_type::iterator iterator;   /* map 是可以修改实值的, 所以不和 set 一样, 使用 const iterator */
    typedef typename rep_type::const_iterator const_iterator;
    typedef typename rep_type::size_type size_type;
    typedef typename rep_type::difference_type difference_type;

    map() : t_(Compare()) {}
    explicit map(const Compare& comp) : t_(comp) {}

    key_compare key_comp() const
    {
        return t_.key_compare();
    }

    value_compare value_comp() const
    {
        return value_compare(t_.key_compare());
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

    Value& operator[](const key_type& x)
    {   
        /* 插入相同的元素, 返回的迭代器指向的是旧值位置!!! 所以可以通过这种方式找到所需的节点, 妙啊 */
        return (*((insert(value_type(x, Value()))).first)).second;
    }

    std::pair<iterator, bool> insert(const value_type& v)
    {   /* 不能有重复数据 */
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

#endif	/* !__WKANGK_STL_MAP_H__ */
