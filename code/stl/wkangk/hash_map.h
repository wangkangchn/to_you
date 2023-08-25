/***************************************************************
 * @copyright  Copyright © 2023 wkangk.
 * @file       hash_map.h
 * @author     wkangk <wangkangchn@163.com>
 * @version    v1.0
 * @brief      基于 hashtable 实现 map
 * @date       2023-08-25 22:03
 **************************************************************/
#ifndef __WKANGK_STL_HASH_MAP_HPP__ 
#define __WKANGK_STL_HASH_MAP_HPP__ 
#include <functional>

#include "hash_table.h"
#include "common.h"


namespace wkangk_stl {

template <typename Key, typename Value, typename HashFcn=std::hash<Value>,
            typename EqualKey = equal_to<Value>,
            typename Alloc=alloc>
class hash_map
{   
    /* map 的键值是不一样的
    实值存储为 pair 的形式是为了能够从实值中获得键值!                       从实值中提取键                    键相等的比较  */
    typedef hash_table<Key, std::pair<const Key, Value>, HashFcn, select1st<std::pair<const Key, Value>>, EqualKey, Alloc> ht;

public:
    typedef typename ht::key_type key_type;
    typedef typename ht::value_type value_type;
    typedef typename ht::hasher hasher;
    typedef typename ht::key_equal key_equal;
    
    typedef typename ht::size_type size_type;
    typedef typename ht::difference_type difference_type;

    /* 不能允许在迭代的过程中修改 set 中值, 因为这会破坏元素在 hash 表中的规则 */
    typedef typename ht::const_pointer pointer;
    typedef typename ht::const_pointer const_pointer;
    typedef typename ht::const_reference reference;
    typedef typename ht::const_reference const_reference;
    typedef typename ht::const_iterator iterator;
    typedef typename ht::const_iterator const_iterator;

    hash_map() : rep_(100, hasher(), key_equal()) 
    {
    }

    hash_map(size_t n) : rep_(n, hasher(), key_equal())
    {
    }

public:
    /* hash_map 和 hash_map 都是配接器, 获取其所有方法都可以基于底层数据结构进行操作 */
    size_type size() const { return rep_.size(); }
    size_type max_size() const { return rep_.max_size(); }
    bool empty() const { return rep_.empty(); }

    iterator begin() const { return rep_.begin(); }
    iterator end() const { return rep_.end(); }

    /* 这个版本是不允许重复插入的 */
    std::pair<iterator, bool> insert(const value_type& v)
    {
        return rep_.insert_unique(v);
    }

    size_type count() const
    {
        return rep_.count();
    }

    void clear()
    {
        rep_.clear();
    }

    void resize(size_type n)
    {
        rep_.resize(n);
    }


private:
    ht rep_;
};

}
#endif	/* !__WKANGK_STL_HASH_MAP_HPP__ */
