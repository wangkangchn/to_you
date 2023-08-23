/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: vector.h
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: vector
时间	   	: 2023-06-06 21:09
***************************************************************/
#ifndef __WKANGK_STL_VECTOR_HPP__ 
#define __WKANGK_STL_VECTOR_HPP__ 
#include <algorithm>

#include "config.h"
#include "alloc.h"
#include "construct.h"
#include "uninitialized.h"


__WKANGK_STL_BEGIN_NAMESPACE


/* vector 支持动态增长的线性数组, 当现有的存储空间不足时, 
会以原先大小的 2 倍进行扩充 */
template <typename T, typename Alloc = alloc>
class vector
{
public:
    typedef T               value_type;
    typedef value_type*     pointer;
    typedef value_type*     iterator;       /* 因为 vector 本质上就是数组, 所以它的迭代器就完全可以使用指针 */
    typedef const iterator     const_iterator;       

    typedef value_type&     reference;       
    typedef const reference      const_reference;       
    typedef size_t          size_type;       
    typedef ptrdiff_t       difference_type;

public:
    vector() : start_(nullptr), finish_(nullptr), end_of_storage_(nullptr) {}
    explicit vector(size_type n)
    {
        fill_initialize(n, T());        /* 从这里就可以看出, 当仅传入一个大小时, 所保存的
                                        类必须支持默认构造 */
    }
    vector(size_type n, const value_type& value)
    {
        fill_initialize(n, value);        
    }

    vector(int n, const value_type& value)
    {
        fill_initialize(n, value);        
    }

    vector(long n, const value_type& value)
    {
        fill_initialize(n, value);
    }

    vector(const_iterator first, const_iterator last) 
    {
        size_type n = 0;
        distance(first, last, n);
        start_ = allocate_and_copy(n, first, last);
        finish_ = start_ + n;
        end_of_storage_ = finish_;
    }

    ~vector()
    {
        destroy(start_, finish_);   /* 先析构 */
        deallocate();               /* 再释放内存 */
    }

public:
    /* 下面两个 const 也很重要, operator[] const 只能访问 const 方法
    也就只能访问这两个, 没有它们的话就会报错 ‘this’ argument discards qualifiers */
    const_iterator begin() const { return start_; }
    const_iterator end() const { return finish_; }
    
    iterator begin()    { return start_; }
    iterator end()    { return finish_; }
    size_type size() const { return static_cast<size_type>(finish_ - start_); }      
    /* 为什么用成员方法而不用数据成员? 
    因为 const 方法不能调用非 const 方法 */
    size_type capacity() const { return static_cast<size_type>(end_of_storage_ - start_); }
    bool empty() const { return finish_ == start_; }
    reference operator[](size_type n) { return *(begin() + n); }
    const_reference operator[](size_type n) const { return *(begin() + n); }
    reference front() { return *begin(); }
    reference back() { return *(end() - 1); }   /* 区间为左闭右开 */
    void push_back(const value_type& value)
    {
        if (finish_ != end_of_storage_) {   /* 满了, 就需要进行扩充 */
            construct(finish_, value);
            ++finish_;
        } else {
            insert_aux(end(), value);
        }
    }

    void pop_back()
    {
        --finish_;
        destroy(finish_);
    }

    /* 返回删除位置 */
    iterator erase(iterator position)
    {
        if (position + 1 != end()) {
            std::copy(position + 1, end(), position);
        }
        --finish_;
        destroy(finish_);
        return position;
    }

    iterator erase(iterator first, iterator last)
    {   
        auto it = std::copy(last, end(), first);    /* 返回拷贝的尾指针 */
        destroy(it, finish_);
        finish_ = finish_ - (last - first);
        return first;
    }

    void clear()
    {
        erase(begin(), end());
    }

    void insert(iterator postion, size_type n, const value_type& value);

    void swap(vector<T, Alloc>& x) 
    {
        std::swap(start_, x.start_);
        std::swap(finish_, x.finish_);
        std::swap(end_of_storage_, x.end_of_storage_);
    }
    

    void reserve(size_type n) 
    {
        if (capacity() < n) {
            const size_type old_size = size();
            iterator tmp = allocate_and_copy(n, start_, finish_);
            destroy(start_, finish_);
            deallocate();
            start_ = tmp;
            finish_ = tmp + old_size;
            end_of_storage_ = start_ + n;
        }
    }

private:
    typedef simple_alloc<value_type, Alloc> data_allocator;

    /* 使用数据填充指定个数个数据 */
    void fill_initialize(size_type n, const T& value)
    {
        start_ = allocate_and_fill(n, value);
        end_of_storage_ = start_ + n;
        finish_ = start_ + n;
    }

    /* 先分配内存再填充 */
    iterator allocate_and_fill(size_type n, const T& value)
    {
        auto result = data_allocator::allocate(n);
        return uninitialized_fill_n(result, n, value);
    }

    /* 释放内存 */
    void deallocate()
    {
        if (start_) {
            data_allocator::deallocate(start_, capacity());
            start_ = finish_ = end_of_storage_ = nullptr;
        }
    }

    void insert_aux(iterator position, const value_type& value);


    iterator allocate_and_copy(size_type n, const_iterator first, const_iterator last) 
    {
        iterator result = data_allocator::allocate(n);
        uninitialized_copy(first, last, result);
        return result;
    }

private:
    iterator    start_;         /* 已用空间的起始位置 */
    iterator    finish_;        /* 已用空间的结束位置 */
    iterator    end_of_storage_; /* 可用空间的结束位置, start 固定不动 */
};


template <typename T, typename Alloc>
void vector<T, Alloc>::insert_aux(vector<T, Alloc>::iterator position, const vector<T, Alloc>::value_type& value)
{
    if (finish_ != end_of_storage_) {    /* 还有空间, 插入 */
        /* 现在末尾构造一个对象 */
        construct(vector<T, Alloc>::end(), vector<T, Alloc>::back());
        ++finish_;

        /* 将 position 处以及后面的数据向后移动 */
        /* 为何 - 2? 因为数据区间是左闭右开, 所以需要 -1, 而前面 finish 又 ++, 所以还需要再
        -1 */
        std::copy_backward(position, vector<T, Alloc>::end() - 2, vector<T, Alloc>::end() - 1);

        // *position = value;           /* 不能直接赋值, const 无法赋值给非 const */
        vector<T, Alloc>::value_type value_copy = value;
        *position = value_copy;

    } else {    /* 没有空间了, 就先分配原先空间 2 倍的大小, 而后将数据依次拷贝到新内存中 */

        size_type old_size = size();
        size_type new_size = old_size != 0 ? old_size * 2 : 1;  // 这里是元素个数
        auto new_start = data_allocator::allocate(new_size);
        iterator new_finish = new_start;

        try {
            new_finish = uninitialized_copy(begin(), position, new_finish);
            construct(new_finish, value);
            ++new_finish;   /* 为何非得在这 ++, 而不在下面 +1
                            为了方式下面抛出异常时, 能够正确处理构造完成的对象 */
            uninitialized_copy(position, end(), new_finish);
        } catch (...) {
            destroy(new_start, new_finish);
            data_allocator::deallocate(new_start, new_size);
        }

        /* 释放原始内存 */
        destroy(begin(), end());
        deallocate();

        start_ = new_start;
        finish_ = new_finish;
        end_of_storage_ = start_ + new_size;
    }
}


template <typename T, typename Alloc>
void vector<T, Alloc>::insert(iterator position, size_type n, const value_type& value)
{
    if (n != 0) {
        if (static_cast<size_type>(end_of_storage_ - finish_) >= n) {    /* 余量还够 */
            value_type value_copy = value;
            const size_type elems_after = finish_ - position;   
            iterator old_finish = finish_;
            if (elems_after > n) {  /* 插入点之后现有元素的数目 > 新增元素个数 */
                /* 先拷贝多出来的, 再拷贝前面, 最后填充 */
                uninitialized_copy(finish_ - n, finish_, finish_);
                finish_ += n;
                std::copy_backward(position, old_finish - n, old_finish);
                std::fill(position, position + n, value_copy);
            } else {    
                /* 分开处理有什么特别的吗? 不理解 */
                uninitialized_fill_n(finish_, n - elems_after, value_copy);
                finish_ += n - elems_after;
                uninitialized_copy(position, old_finish, finish_);
                finish_ += elems_after;
                std::fill(position, old_finish, value_copy);
            }
        } else {
            /* 同样, 少了就变为旧长度的两倍 */
            const size_type old_size = size();
            const size_type len = old_size + std::max(old_size, n);

            iterator new_start = data_allocator::allocate(len);     /* 开辟新内存 */
            iterator new_finish = new_start;

            try {
                /* 拷贝插入前 */
                new_finish = uninitialized_copy(start_, position, new_start);

                /* 要插入的数据 */
                new_finish = uninitialized_fill_n(new_finish, n, value);

                /* 后半部分 */
                new_finish = uninitialized_copy(position, finish_, new_finish);
            } catch (...) {
                destroy(new_start, new_finish);
                data_allocator::deallocate(new_start, len);
                throw;
            }

            /* 删除原来的内存 */
            destroy(start_, finish_);
            deallocate();

            start_ = new_start;
            finish_ = new_finish;
            end_of_storage_ = new_start + len;
        }
    }
}

__WKANGK_STL_END_NAMESPACE

#endif	/* !__WKANGK_STL_VECTOR_HPP__ */