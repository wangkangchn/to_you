/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: deque.h
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 双向队列
时间	   	: 2023-07-25 20:39
***************************************************************/
#ifndef __WKANGK_STL_DEQUE_H__ 
#define __WKANGK_STL_DEQUE_H__ 
#include <stdint.h>

#include "config.h"
#include "alloc.h"


__WKANGK_STL_BEGIN_NAMESPACE



/**
 *     计算缓冲区大小
 *
 * @param [in]  n       期望缓冲区字节数
 * @param [in]  sz      一个元素的字节数
 * @return     总的缓冲区字节数(也就是一段内存的大小)
 */
size_t __deque_buf_size(size_t n, size_t sz) 
{
    /* 不为 0 就使用用户设置的值
    为 0, 当元素小于 512, 就分配 512 / sz, 不是很理解为啥要这么搞 */
    return n != 0 ? n : ( sz < 512 ? size_t(512 / sz) : size_t(1) );
}

template <typename T, typename Ref, typename Ptr, size_t BufSiz>
class __deque_iterator
{
public:
    typedef __deque_iterator<T, T&, T*, BufSiz>     iterator;
    typedef __deque_iterator<T, const T&, const T*, BufSiz>     const_iterator;
    static size_t buffer_size() { return __deque_buf_size(BufSiz, sizeof(T)); }

    /* 实现迭代器的五个类型 */
    typedef random_access_iterator_tag      iterator_category;
    typedef T                               value_type;
    typedef Ptr                             pointer;
    typedef Ref                             reference;
    typedef size_t                          size_type;
    typedef ptrdiff_t                       difference_type;

    typedef T**                             map_pointer;
    typedef __deque_iterator                self;


public:
    reference operator*() const
    {
        return *cur_;
    }

    pointer operator->() const  /* 取得具体类型的指针 */
    {
        return &(operator*());
    }

    /* this - x */
    difference_type operator-(const self& x) const
    {
        /* 为何要 -1, 因为如果两个迭代器紧挨着就不需要加一个缓冲区长度
        两个迭代器在两个缓冲区中都有数据
        下面这个方法, this 比 x 大没问题, 但是 this 比 x 小为啥也可以这么算?
        小于的时候还是假设大于, 求了一个个数, 然后用 node 取正负, 我是这么理解的, 哈哈
        好像不太对
         */
        return difference_type(buffer_size()) * (node_ - x.node_ - 1) 
                    + (cur_ - first_)       /* */
                    + (x.last_ - x.cur_);
    }

    /* 前 ++ */
    self& operator++()
    {
        ++cur_;
        if (cur_ == last_) {          /* last_ 不指向元素 */
            set_node(node_ + 1);
            cur_ = first_;
        }
        return *this;
    }

    self operator++(int)
    {
        self tmp = *this;
        ++(*this);      /* 借助前 ++ 实现*/
        return tmp;
    }

    self& operator--()
    {
        if (cur_ == first_) {          /* last_ 不指向元素 */
            set_node(node_ - 1);
            cur_ = last_;
        }
        --cur_;
        return *this;
    }

    self operator--(int)
    {
        self tmp = *this;
        --(*this);      /* 借助前 ++ 实现*/
        return tmp;
    }


    /* 实现随机存取 */
    self& operator+=(difference_type n)
    {
        difference_type offset = n + (cur_ - first_);

        if (offset >= 0 && offset < difference_type(buffer_size())) {
            cur_ += n;  /* 还在一个缓冲区 */
        } else {
            /* 不再一个缓冲区了, 就要调整所有指针了 */
            /* 计算 node 偏移 */
            difference_type node_offset = offset > 0 ? offset / difference_type(buffer_size()) 
                    : -difference_type((-offset - 1) / buffer_size()) - 1;
            /* 
                两个 -1 不是很明白
                如果 offset == -513
                    -offset - 1 == 512
                    -1 - 1 = -2

                    直接除 512
                    -512 -1
                    -513 -1 实际应该是 -2
                    -514 -1 实际应该是 -2
                    应该就是为了解决这种问题.
                    在边界的时候先让你小一点, 计算完了再恢复
             */
            set_node(node_ + node_offset);
            cur_ = first_ + (offset - node_offset * difference_type(buffer_size));
        }

        return *this;
    }

    self operator+=(difference_type n) const
    {
        self tmp = *this;
        return tmp += n;
    }

    self& operator-=(difference_type n)
    {
        return *this += (-n);
    }

    self operator-=(difference_type n) const
    {
        self tmp = *this;
        return tmp -= n;
    }

    reference operator[](difference_type n) const
    {
        return *(*this += n);   /* 这里要直接返回元素值, 所以最外面还有一个 *  */
    }

    bool operator==(const self& x) const
    {
        return cur_ == x.cur_;
    }

    bool operator!=(const self& x) const
    {
        return !(*this == x);
    }

    bool operator<=(const self& x) const
    {
        return node_ == x.node_ ? (cur_ < x.cur_) : (node_ < x.node_);
    }

public:
    /**
     *     设置使用新节点
     */
    void set_node(map_pointer new_node)
    {
        node_ = new_node;       /* 对吧 node_ 就是指向 map 中 node 的位置 */
        first_ = *new_node;
        last_ = first_ + difference_type(buffer_size());
    }

public:
    T* cur_;                /* 迭代器指向的当前元素 */ 
    T* first_;              /* 当前元素所在段的起始地址 */
    T* last_;               /* 当前元素所在段的结束地址 */
    map_pointer node_;      /* 当前元素所在段在 map 中的位置 */
};


/**
 *     
 * @param T         存储类型
 * @param Alloc     内存分配器
 * @param BufSize   每一段内存的大小, 单位字节数
 */
template <typename T, typename Alloc=alloc, size_t BufSize=0>
class deque
{
public:
    typedef T               value_type;
    typedef value_type*     pointer;
    typedef value_type&     reference;
    typedef const value_type&     const_reference;
    typedef pointer*        map_pointer;
    typedef size_t          size_type;
    typedef ptrdiff_t       difference_type;

    typedef __deque_iterator<T, T&, T*, BufSize> iterator;

    typedef simple_alloc<value_type, Alloc> data_allocator;     /* 元素分配器 */
    typedef simple_alloc<pointer, Alloc> map_allocator;     /* 分配 map 空间 */

public:
    deque() : 
        start_(), finish_(), map_(nullptr), map_size_(0)
    {
        create_map_and_nodes(0);
    }

    /**
     * @param [in]  n 初始元素个数
     */
    deque(int n, const value_type& value) :
        start_(), finish_(), map_(0), map_size_(0)
    {
        fill_initialize(n, value);
    }

    ~deque() 
    {
        destroy(start_, finish_);
        destroy_map_and_nodes();
    }

public:
    iterator begin() 
    {
        return start_;
    }

    iterator end() 
    {
        return finish_;
    }

    reference operator[](size_type n)
    {
        return start_[difference_type(n)];
    }

    reference front()
    {
        return *start_;
    }

    reference back()
    {
        // return *(finish_ - 1);  /* 看侯捷老师说的, 我也觉得可以直接这么搞, iterator 定义了 - 运算符, 应该可以吧, 试试 */

        /* 不对不对 不能上面这么写, 上面只定义了 两个 iterator 之间的 -, 而没有定义 iterator 与 整数之间的减,
        但不是有 -=, -= 也不行??? 基础不牢! 我记得有一个匹配顺序, 但是记不清楚了 */
        // return *(finish_ - 1);   /* 不可不可, 出错, 不行 -= 不行, 必须完全匹配 */
        iterator tmp = finish_;
        --tmp;
        return *tmp;
    }

    size_type size()
    {
        return finish_ - start_;    /* 两个 ;; 就是写错了吧 /捂脸 */
    }

    size_type max_size() const 
    {
        return size_type(-1);   /* 这个学到了, -1 补码 11111111, 转为无符号数, 就是该类型的最大值! */
    }

    bool empty() const
    {
        return finish_ == start_;
    }

    /* 尾插 */
    void push_back(const value_type& v)
    {
        if (finish_.cur_ != finish_.last_ - 1) { /* 还有空间, 就存入元素 */
            construct(finish_.cur_, v);
            ++finish_.cur_;
        } else {
            /* 没有空间了就要重新分配 */
            push_back_auc(v);
        }
    }

    /* 尾删 */
    void pop_back()
    {
        if (finish_.cur_ != finish_.first_) {
            /* 当前段内还有一些元素, 直接删除即可 */
            --finish_.cur_;
            destroy(finish_.cur_);  /* 左闭右开 */
        } else {
            /* 当前正好是当前段的边界上, 这样就需要把当前段排除在外, 需要删掉? 我感觉没啥必要吧 */
            pop_back_aux();
        }
    }

    /* 头删 */
    void pop_front()
    {
        if (start_.cur_ != start_.last_ - 1) {
            destroy(start_.cur_);
            ++start_.cur_;
        } else {
            pop_front_aux();
        }
    }

private:
    static size_type buffer_size() 
    {
        return __deque_buf_size(BufSize, sizeof(value_type));
    }

    /**
     *     分配空间填充数据
     *
     * @param [in]  n       元素个数
     * @param [in]  value   初值
     */
    void fill_initialize(size_type n, const value_type& value)
    {
        create_map_and_nodes(n);
        map_pointer cur;
        // try {    不做异常处理了

        for (cur = start_.node_; cur != finish_.node_; ++cur) {
            uninitialized_fill(*cur, *cur + buffer_size(), value);
        }
        /* 这里书中说的是, 尾端可能有备用空间, 不必设初值, 不是很理解, 很奇怪 */
        uninitialized_fill(finish_.first_, finish_.cur_, value);
        // }
    }

    /**
     *  在 map 中分配 num_elements 个段
     * @param [in]  num_elements    元素个数而不是字节个数, map 段是记录了元素个数
     */
    void create_map_and_nodes(size_type num_elements) 
    {
        /* 需要管理的内存段, 当 num_elements 正好被 buffer_size() 整除时, 会
        多分配一个节点 */
        size_type num_nodes = num_elements / buffer_size() + 1;

        /* map 有一个最小值, 最小管理 8 个内存段 */
        map_size_ = std::max(initial_map_size(), num_nodes + 2);

        map_ = map_allocator::allocate(map_size_);

        /* 初始使头尾指向最中间, 以使两边的扩增均匀 */
        map_pointer nstart = map_ + (map_size_ - num_nodes) / 2;
        map_pointer nfinish = nstart + num_nodes - 1;

        map_pointer cur;
        /* 为每个内存段分配缓冲区 */
        for (cur = nstart; cur <= nfinish; ++cur) {
            *cur = allocate_node();
        }
        
        start_.set_node(nstart);
        finish_.set_node(nfinish);
        start_.cur_ = start_.first_;
        /* 最后令 finish 指向最后一个节点的下一个位置, 满足 STL 左闭右开的特点 */
        finish_.cur_ = finish_.first_ + num_elements % buffer_size();

    }

    /* 分配空间, 设置值, 调整指针 */
    void push_back_auc(const value_type& v)
    {
        value_type v_copy = v;
        reserver_map_at_back();
        *(finish_.node_ + 1) = allocate_node();

        construct(finish_.cur_, v_copy);
        finish_.set_node(finish_.node_ + 1);
        finish_.cur_ = finish_.first_;
    }

    static size_type initial_map_size() { return 8; }
    pointer allocate_node() { return data_allocator::allocate(buffer_size()); }

    /**
     *  在 map 的后面添加新的结点     
     */
    void reserver_map_at_back(size_type nodes_to_add=1)
    {
        /* map 尾部没有空间了, finish_ 是最后一个使用的节点, 因为
        这里是后面添加, 所以只考虑尾部 map 的余量 */
        if (nodes_to_add + 1 > map_size_ - (finish_.node_ - map_)) {
            reallocate_map(nodes_to_add, false);
        }
    }

    /**
     * 因为 deque 是前后都可以操作, 所以这里有第二个参数, 用来表示
     * 添加的新空间是在之前添加还是在之后添加
     */
    void reallocate_map(size_type nodes_to_add, bool add_at_front)
    {
        size_type old_num_nodes = finish_.node_ - start_.node_ + 1;
        size_type new_num_nodes = old_num_nodes + nodes_to_add;

        map_pointer new_nstart;
        if (map_size_ > 2 * new_num_nodes) {        /* map 还有充足的空间可以容纳新空间, 所以就可以使用之前分配的空间 */
            /* 重新调整所有内容的位置 */
            new_nstart = map_ + (map_size_ - new_num_nodes) / 2 + (add_at_front ? nodes_to_add : 0);
            if (new_nstart < start_.node_) {
                /* 重新调整后起始位置可能会到之前, 因为这里调整的时候, 是在原先 map
                空间中, 再找一个中间位置放进去 */
                std::copy(start_.node_, finish_.node_ + 1, new_nstart); /* 对啊吧 是整体移动 */
            } else {
                /* ooo copy_backward 是倒着复制, 先复制最后一个, 而后向前然后复制前面的东西
                    为什么要这么处理? 因为现在 new_nstart 在 start_.node_ 之后, 可能存在
                    交叉的情况, 这时使用 copy, 就可能会出错. 所以使用先复制最后的元素, 
                    这样即时被覆盖了, 也可以保留前的元素 */
                std::copy_backward(start_.node_, finish_.node_ + 1, new_nstart + old_num_nodes);
            }
        } else {
            /* 空间不够了, 就重新分配. 同样每次增加时, 还是有一个最小大小限制 */
            size_type new_map_size = map_size_ + std::max(map_size_, nodes_to_add) + 2;
            map_pointer new_map = map_allocator::allocate(new_map_size);
            /* 同样新位置在新 map 中间, 使用 add_at_front 表明向前扩展一个 */
            new_nstart = new_map + (new_map_size - new_num_nodes) / 2 + (add_at_front ? nodes_to_add : 0);

            /* 原 map 内容拷贝新 map 中 */
            std::copy(start_.node_, finish_.node_ + 1, new_nstart);
            map_allocator::deallocate(map_, map_size_); /* 要记得删除之前的元素 */
            map_ = new_nstart;
            map_size_ = new_map_size;
        }

        start_.set_node(new_nstart);
        finish_.set_node(new_nstart + old_num_nodes - 1);
    }

    void destroy_map_and_nodes() 
    {
        for (map_pointer cur = start_.node_; cur <= finish_.node_; ++cur) {
            deallocate_node(*cur);
        }
        map_allocator::deallocate(map_, map_size_);
    }

    void deallocate_node(pointer n) 
    {
        data_allocator::deallocate(n, buffer_size());
    }

    void pop_back_aux()
    {   
        /* 没必要删掉吧, 直接调整指针不行吗? */
        deallocate_node(finish_.first_);
        finish_.set_node(finish_.node_ - 1);
        finish_.cur_ = finish_.last_ - 1;
        destroy(finish_.cur_);      /* 左闭右开 */
    }

    void pop_front_aux()
    {
        destroy(start_.cur_);
        deallocate_node(start_.first_);
        start_.set_node(start_.node_ + 1);
        start_.cur_ = start_.first_;
    }

private:
    iterator start_;        /* map 中使用的第一个节点 */
    iterator finish_;       /* map 中使用的最后一个节点 */

    map_pointer map_;       /* 类似与页表存储一段一段的内存 */
    size_type map_size_;    /* map 的大小 */
};


__WKANGK_STL_END_NAMESPACE
#endif	/* !__WKANGK_STL_DEQUE_H__ */
