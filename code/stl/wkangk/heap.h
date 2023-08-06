/***************************************************************
 * @copyright  Copyright © 2023 wkangk.
 * @file       heap.h
 * @author     wkangk <wangkangchn@163.com>
 * @version    v1.0
 * @brief      堆
 *      堆没有提供具体的数据结构类, 而只是提供了几个辅助函数, 
 * 因为堆可以建立在任何支持随机访问的容器之上, 所以就没有具体类.
 * 但是 stack 和 queue 也是一样的类似情况, 为啥它们就有自己的类?
 * 应该是设计问题, 我还太嫩, 接触不到 -.-
 * @date       2023-08-05 08:26
 **************************************************************/
#ifndef __WKANGK_STL_HEAP_H__ 
#define __WKANGK_STL_HEAP_H__ 
#include "config.h"
#include "iterator.h"


__WKANGK_STL_BEGIN_NAMESPACE



/**
 *
 * @param [in]  first           堆开始位置
 * @param [in]  hold_index      待调整位置索引
 * @param [in]  top_index       堆顶位置索引
 * @param [in]  value           待插入的值
 */
template <typename RandomAccessIterator, typename Distance, typename T, class Compare>
void __push_heap(RandomAccessIterator first, Distance hold_index, Distance top_index, T value, Compare comp)
{
    /* 1. 先要找到父节点, 因为是以完全二叉树为基准, 所以 父 = 子 / 2 (向下取整) */
    Distance parent = (hold_index - 1) / 2;     /* 这里为啥要 -1? 猜测可能时代调整元素是实际元素的下一个位置, 左闭右开的右开 */

    /*          判断是否找到堆顶              判断父值是否比当前元素小(大根堆) */
    // while ( (hold_index > top_index) && ( *(first + parent) < value ) ) {
    while ( (hold_index > top_index) && comp( *(first + parent), value ) ) {
        /* 父值下落 */
        *(first + hold_index) = *(first + parent);      /* 这里 hold 没有 -1, 好像前面的猜测不太对 */

        hold_index = parent;
        parent = (hold_index - 1) / 2;          /* 为何要 -1 ???? */
    }

    /* 持续到堆顶, 或找到了父值大的情况, 就把元素存入当前位置 */
    *(first + hold_index) = value;
}


template <typename RandomAccessIterator, class Compare, typename Distance, typename T>
void __push_heap_aux(RandomAccessIterator first, RandomAccessIterator last, Compare comp, Distance*, T*)
{
    wkangk_stl::__push_heap(first, Distance((last - first) - 1), Distance(0), T(*(last - 1)), comp);
}

/* 插入元素, 仅接受随机存取迭代器
ooo, 为啥 heap 进行提供函数? 因为它只需要操作迭代器就好, 与具体的
容器没有关系. 可以可以, 而 stack 与 queue 是基于容器的, 与迭代器没
有关系, 要用到容器中的方法, 所以在 stack 和 queue 内部需要保存所依
赖的底层容器

    调用此方法的前提时, 数据已经存放到迭代器的最末端元素, 这个函数
的功能仅仅是进行堆属性的调整

@param first 堆顶
@param last  堆底, 真实元素的下一位置, 左闭右开
 */
template <typename RandomAccessIterator, class Compare>
void push_heap(RandomAccessIterator first, RandomAccessIterator last, Compare comp)
{
    /* 后面两个参数仅仅是为了获取类型, 因为随机存取器, 前后移动可能会有不同类型, 不是简单的 ptrdiff_t */
    wkangk_stl::__push_heap_aux(first, last, comp, distance_type(first), value_type(first));
}



/**
 *     
 * @param [in]  first           堆头
 * @param [in]  hold_index      要调整的节点索引
 */
template <typename RandomAccessIterator, typename Distance, typename T, class Compare>
void __adjust_heap(RandomAccessIterator first, Distance hold_index, Distance len, T value, Compare comp)
{
    Distance top_index = hold_index;
    Distance second_child = hold_index * 2 + 2; /* 先看右节点, 为什么是 + 2, 不是 + 1, 很奇怪呀, 前面也是会 -1 奇怪奇怪
                                                我知道为啥 -2 了, -1 的情况是从索引 1 开始计算的, STL 中使用的是从索引 0 开始的!!! */
    while (second_child < len) {
        // if (*(first + second_child) < *(first + (second_child - 1))) {  /* 左比右大 */
        if (comp(*(first + second_child), *(first + (second_child - 1)))) {  /* 左比右大 */
            second_child--;
        }

        *(first + hold_index) = *(first + second_child);        /* 因为堆顶是最小的, 所以这里和最大的节点交换 */
        hold_index = second_child;
        second_child = second_child * 2 + 2;                    /* 找到当前节点的子节点 */
    }

    if (second_child == len) {      /* 只有左子树 */
        *(first + hold_index) = *(first + (second_child - 1));
        hold_index = second_child - 1;
    }

    wkangk_stl::__push_heap(first, hold_index, top_index, value, comp);       /* 调整完局部后, 可能上面的还不满足, 所以需要在上溯一次!!! */
}

/* 模板参数声明的顺序并没有什么关系, 可以随意放位置 */
template <typename RandomAccessIterator, typename Distance, typename T, class Compare>
void __pop_heap(RandomAccessIterator first, RandomAccessIterator last, RandomAccessIterator result, T value, Compare comp, Distance*)
{
    *result = *first;   /* 要取出的值放在最后 */
    wkangk_stl::__adjust_heap(first, Distance(0), Distance(last - first), value, comp);
}


template <typename RandomAccessIterator, typename T, class Compare>
void __pop_heap_aux(RandomAccessIterator first, RandomAccessIterator last, T*, Compare comp)
{
    wkangk_stl::__pop_heap(first, last - 1, last - 1, T(*(last - 1)), comp, distance_type(first));
}


template <typename RandomAccessIterator, class Compare>
void pop_heap(RandomAccessIterator first, RandomAccessIterator last, Compare comp)
{
    wkangk_stl::__pop_heap_aux(first, last, value_type(first), comp);
}


template <typename RandomAccessIterator, typename T, typename Distance, typename Compare>
void __make_heap(RandomAccessIterator first, RandomAccessIterator last, Compare comp, T*, Distance*)
{
    /* 元素是 0 1, 不必重排, 左闭右开 */
    if ((last - first) < 2) {
        return;
    }

    Distance len = last - first;

    /* 找到最后一个叶节点的根节点, 然后向下调整 */
    Distance top_index = (len - 2) / 2;
    
    while (true) {
        wkangk_stl::__adjust_heap(first, top_index, len, T(*(first + top_index)), comp);
        if (top_index == 0) {   /* 可能一次就调整到了堆顶 */
            return;
        }

        --top_index;            /* 不能调整到堆顶, 就从另一个节点继续调整!!! 可以可以 */
    }
}


template <typename RandomAccessIterator, typename Compare>
void make_heap(RandomAccessIterator first, RandomAccessIterator last, Compare comp)
{
    wkangk_stl::__make_heap(first, last, comp, value_type(first), difference_type(first));
}

template <class RandomAccessIterator, class Compare>
void sort_heap(RandomAccessIterator first, RandomAccessIterator last, Compare comp)
{
    /* 排序操作就是把所有的堆顶一个个弹出来就好了 */
    while (last - first > 1) {
        wkangk_stl::pop_heap(first, last--, comp);
    }
}



/* -------------------------------------------------------------------------------
 * 默认最大堆
 * ------------------------------------------------------------------------------- */
template <typename T>
struct default_compare
{
    typedef std::less<T> Compare;       /* 默认按最大堆构建 */
};

template <typename RandomAccessIterator>
void make_heap(RandomAccessIterator first, RandomAccessIterator last)
{
    typedef typename default_compare<typename iterator_traits<RandomAccessIterator>::value_type>::Compare  Compare;
    wkangk_stl::make_heap(first, last, Compare());
}

template <typename RandomAccessIterator>
void push_heap(RandomAccessIterator first, RandomAccessIterator last)
{
    typedef typename default_compare<typename iterator_traits<RandomAccessIterator>::value_type>::Compare  Compare;
    wkangk_stl::push_heap(first, last, Compare());
}


template <typename RandomAccessIterator>
void pop_heap(RandomAccessIterator first, RandomAccessIterator last)
{   
    typedef typename default_compare<typename iterator_traits<RandomAccessIterator>::value_type>::Compare  Compare;
    wkangk_stl::pop_heap(first, last, Compare());
}

template <class RandomAccessIterator>
void sort_heap(RandomAccessIterator first, RandomAccessIterator last)
{
    typedef typename default_compare<typename iterator_traits<RandomAccessIterator>::value_type>::Compare  Compare;
    wkangk_stl::sort_heap(first, last, Compare());
}

__WKANGK_STL_END_NAMESPACE

#endif	/* !__WKANGK_STL_HEAP_H__ */
