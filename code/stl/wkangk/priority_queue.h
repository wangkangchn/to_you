/***************************************************************
 * @copyright  Copyright © 2023 wkangk.
 * @file       priority_queue.h
 * @author     wkangk <wangkangchn@163.com>
 * @version    v1.0
 * @brief      优先级队列
 * @date       2023-08-06 09:15
 **************************************************************/
#ifndef __WKANGK_STL_PRIORITY_QUEUE_H__ 
#define __WKANGK_STL_PRIORITY_QUEUE_H__ 
#include <functional>

#include "config.h"
#include "heap.h"
#include "vector.h"


__WKANGK_STL_BEGIN_NAMESPACE


template <typename T, typename Sequence=vector<T>, typename Compare=std::less<typename Sequence::value_type>>
class priority_queue
{
public:
    typedef typename Sequence::value_type   value_type;
    typedef typename Sequence::size_type    size_type;
    typedef typename Sequence::reference    reference;
    typedef typename Sequence::const_reference    const_reference;

public:
    priority_queue() :
        c_()
    {
    }

    explicit priority_queue(const Compare& comp) :
        c_(), comp_(comp)
    {
    }

    template <typename InputIterator>
    priority_queue(InputIterator first, InputIterator last, const Compare& comp) :
        c_(first, last), comp_(comp)
    {
        wkangk_stl::make_heap(c_.begin(), c_.end(), comp_);     /* 按照指定的要求建堆 */
    }

    template <typename InputIterator>
    priority_queue(InputIterator first, InputIterator last) :
        c_(first, last)
    {
        wkangk_stl::make_heap(c_.begin(), c_.end(), comp_);     /* 按照指定的要求建堆 */
    }

    bool empty() const 
    {
        return c_.empty();
    }

    size_type size() const 
    {
        return c_.size();
    }

    const_reference top()
    {
        return c_.front();  /* 必须是实现了这写方法的容器才可以用作优先级队列的底层结构 */
    }

    void push(const value_type& x)
    {
        c_.push_back(x);
        wkangk_stl::push_heap(c_.begin(), c_.end(), comp_);
    }

    void pop()
    {
        wkangk_stl::pop_heap(c_.begin(), c_.end(), comp_);
        c_.pop_back();  /* heap 相关只关心堆规则, 具体的删除要底层结构支持 */
    }

private:
    Sequence c_;        /* 底层容器 */
    Compare comp_;      /* 比较运算符 */
};



__WKANGK_STL_END_NAMESPACE

#endif	/* !__WKANGK_STL_PRIORITY_QUEUE_H__ */