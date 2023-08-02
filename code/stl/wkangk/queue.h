/***************************************************************
 * @copyright  Copyright © 2023 wkangk.
 * @file       stack.h
 * @author     wkangk <wangkangchn@163.com>
 * @version    v1.0
 * @brief      队列
 * @date       2023-08-01 20:33
 **************************************************************/
#ifndef __WKANGK_STL_QUEUE_H__ 
#define __WKANGK_STL_QUEUE_H__ 
#include "deque.h"

__WKANGK_STL_BEGIN_NAMESPACE

template <typename T, typename Sequence=deque<T>>
class queue
{
public:
    typedef typename Sequence::value_type value_type;
    typedef typename Sequence::size_type size_type;
    typedef typename Sequence::reference reference;
    typedef typename Sequence::const_reference const_reference;

public:
    bool empty() const
    {
        return c_.empty();
    }

    bool size() const
    {
        return c_.size();
    }

    reference front()
    {
        return c_.front();
    }

    const_reference front() const
    {
        return c_.front();
    }

    reference back()
    {
        return c_.back();
    }

    const_reference back() const
    {
        return c_.back();
    }

    /* 尾插 */
    void push(const value_type& v)
    {
        return c_.push_back(v);
    }

    /* 头删 */
    void pop()
    {
        return c_.pop_front();
    }

private:
    Sequence c_;        /* 底层容器 */

};

__WKANGK_STL_END_NAMESPACE

#endif