/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: alloc.h
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 两级内存分配器
时间	   	: 2023-05-25 20:24
***************************************************************/
#include <stdlib.h>

#include "config.h"

__WKANGK_STL_BEGIN_NAMESPACE


#include <iostream>
#define __THROW_BAD_ALLOC   std::cerr << "out of memory" << std::endl; exit(1)

/* 
    第一级内存分配器, 分配大于 128B  的数据
 */
template <int inst>    /* 这个泛型参数并没有啥用处 */
class __malloc_alloc_template
{
public:
    static void* allocate(size_t n) 
    {
        void* result = (void*)malloc(n);
        if (!result) {
            result = oom_malloc(n);         /* 内存不足时, 重新分配 */
        }
        return result;
    }

    static void deallocate(void* p, size_t n)
    {
        free(p);
    }

    static void* reallocate(void* p, size_t new_sz)
    {
        void* result = realloc(p, new_sz);
        if (!result) {
            result = oom_malloc(p, new_sz);         /* 内存不足时, 重新分配 */
        }
        return result;
    }

    static void (* set_malloc_header(void(*f)()))()
    {
        void (*old)() = __malloc_alloc_omm_handler;
        __malloc_alloc_omm_handler = f;
        return old;
    }

private:
    static void* oom_malloc(size_t n)
    {
        void (*my_malloc_handler)() = nullptr;
        void* result = nullptr;

        for (;;) {
            my_malloc_handler = __malloc_alloc_omm_handler;
            if (!my_malloc_handler) {
                __THROW_BAD_ALLOC;
            }
            (*my_malloc_handler)();     /* 内存不足时的回调函数 */
            result = malloc(n)
            if (result) {
                return result;
            }
        }
    }

    static void* oom_realloc(void* p, size_t new_sz)
    {
        void (*my_malloc_handler)() = nullptr;
        void* result = nullptr;

        for (;;) {
            my_malloc_handler = __malloc_alloc_omm_handler;
            if (!my_malloc_handler) {
                __THROW_BAD_ALLOC;
            }
            (*my_malloc_handler)();     /* 内存不足时的回调函数 */
            result = realloc(p, new_sz)
            if (result) {
                return result;
            }
        }
    }

    /* 函数指针 */
    static void (*__malloc_alloc_omm_handler)();
};

template <int inst>
void (*__malloc_alloc_template<inst>::__malloc_alloc_omm_handler)() = nullptr;


typedef __malloc_alloc_template<0> malloc_alloc;


/* 对 malloc_alloc 的简单封装, 使字节分配转为元素分配 */
template <typename T, typename Alloc>
class simple_alloc
{
public: 
    static T* allocate(size_t n)
    {
        return n <= 0 ? 0 : (T*)Alloc::allocate(n * sizeof(T));
    }
    
    static T* allocate()
    {
        return (T*)Alloc::allocate(sizeof(T));
    }

    static void reallocate(T* p, size_t n)
    {
        Alloc::deallocate(p, n * sizeof(T));
    }

    static void reallocate(T* p)
    {
        Alloc::deallocate(p, sizeof(T));
    }
};


/* -------------------------------------------------------------------------------
 * 第二级配置器
 * ------------------------------------------------------------------------------- */

enum { __ALIGN = 8 };         /* free list 中只保存 8 的倍数的内存块 */
enum { __MAX_BYTES = 128 };   /* free list 中保存的最大内存的字节数*/
enum { __NFREELISTS = __MAX_BYTES / __ALIGN }; /* free list 可以管理的内存块种类 */


template <int inst>
class __default_alloc_template
{
public:
    static void* allocate(size_t bytes)
    {   
        if (bytes > __MAX_BYTES) {         /* 大于最大交予第一级配置器 */
            return malloc_alloc::allocate(bytes);
        }

        /* 找到申请内存块所属的 free_list 中的链, 链上有空余就返回
        没有空余就进行内存补充 */
        obj** needed_free_list = free_list + FREELIST_INDEX(bytes);
        /* 为什么可以直接判断? 因为 union 变量地址和其成员地址都是一样的
        union 就是给一块内存分了很多区 */
        if (*needed_free_list) {    
            obj* result = *needed_free_list;
            /* 头结点下移, 因为这里是二级指针, 所以直接解引赋值, 修改的就直接是表头 */
            *needed_free_list = *result->free_list_link;
            return (void*)result;
        }

        return refill(ROUND_UP(bytes));
    }

    static void deallocate(void* p, size_t bytes)
    {
        if (bytes > (size_t)__MAX_BYTES) {      /* 无论分配还是释放, 大于 __MAX_BYTES 就统统交由第一级管理器 */
            malloc_alloc::deallocate(p, bytes);
            return;
        }

        /* 寻找对应的 free_list 节点, 回收内存 */
        obj** needed_free_list = free_list + FREELIST_INDEX(bytes);
        ((obj*)p)->free_list_link = *needed_free_list; /* 头插 */
        *needed_free_list = (obj*)p;
    }

    /* 这个并没有做太多的处理 */
    static void* reallocate(void* p, size_t old_sz, size_t new_sz)
    {   
        /* 都大于 __MAX_BYTES 就全部交由全局函数处理 */
        if (old_sz > (size_t)__MAX_BYTES && new_sz > (size_t)__MAX_BYTES) {
            return realloc(p, new_sz);
        }

        if (ROUND_UP(old_sz) == ROUND_UP(new_sz)) {
            return p;
        }

        /* 使用自身的分配器分配所需内存 */
        void* result = allocate(p, new_sz);
        /* 旧数据拷贝到新数据 */
        size_t copy_sz = old_sz <= new_sz ? old_sz : new_sz;
        memcpy(result, p, copy_sz);
        deallocate(p);  /* 释放旧地址 */
        return result;
    }

private:
    /* 因为 union 共用的是同一块内存, 所以数据的地址
    也就是 obj 的地址 */
    union obj
    {
        union obj* free_list_link;  /* 串联一种内存块链上所有的节点, 最后一个指向 nullptr */
        char client_data[1];        /* 实际的内存数据 */
    };
    

    /**
     *  将传入的大小, 向上对其到 __ALIGN 的倍数, 保证每次
     * 申请的内存大小都是 8 的倍数
     */
    static size_t ROUND_UP(size_t bytes)
    {
        return ((bytes + __ALIGN-1) & ~(__ALIGN-1));
    }
    
    /**
     *     计算传入字节数的大小块位于 free_list 中的哪一部分
     */
    static size_t FREELIST_INDEX(size_t bytes) 
    {
        return ((bytes + __ALIGN-1) / __ALIGN - 1); /* 最后 -1 是因为索引从 0 开始 */
    }

    static void* refill(size_t bytes);
    static char* chunk_alloc(size_t bytes, int& nobjs);

private:
    static char* start_free;
    static char* end_free;                  /* 内存池边界 */
    static obj* free_list[__NFREELISTS];    /* 每一个代表了一种内存类型 */
    static size_t heap_size;                /* 没理解这个要干嘛 */
};

template <int inst>
char* __default_alloc_template<inst>::start_free = nullptr;
template <int inst>
char* __default_alloc_template<inst>::end_free = nullptr;

/* 静态数组的初始化, 确实很新奇 */
template <int inst>
__default_alloc_template<inst>::obj* 
__default_alloc_template<inst>::free_list[__NFREELISTS] = { nullptr };

template <int inst>
size_t __default_alloc_template<inst>::heap_size = 0;

/* 
    free_list 空了之后才会进行填充, 所以到这里的时候 free_list 已经没有数据了
    从内存池中取出 20 个指定大小的内存块, 将其加入 free_list 中 */
template <int inst>
void* __default_alloc_template<inst>::refill(size_t bytes)
{
    int nobjs = 20;

    /* chunk 会返回实际分配到的内存数目 */
    char* chunk = chunk_alloc(bytes, nobjs);
    void* result = (void*)chunk; 
    
    /* 多余一个的时候, 拿出第一个返回, 剩余的加入到 free_list 中
    只分配到一个, 就直接返回了, nobjs 不会 <= 0 */
    if (nobjs > 1) {
        obj** needed_free_list = free_list + FREELIST_INDEX(bytes);

        /* 每块申请得到的内存起始地址, 既做实际的内存地址, 也做链表
        地址, 这也可能是为什么申请字节最少是 8B 的原因, 因为 64 位机
        指针变量一个就是 8B, 如果申请的内存小于 8B, 如果将这个地址转
        成 obj*, 如果一旦解引, 会找到 8 个地址空间的大小, 很显然就越界了
        了不起的设计   */
        obj* current_obj = *needed_free_list;                           /* 这里 current_obj 应该是 nullptr */
        *current_obj = (obj*)(chunk + bytes * 1);
        for (size_t i = 2; i < nobjs; ++i) {
            current_obj->free_list_link = (obj*)(chunk + bytes * i);    /* 0 被直接返回了 */
            current_obj = current_obj->free_list_link;
        }
        current_obj = nullptr;    /* 推出循环后, current_obj 就是最后一个节点的 next, 需要赋值 nullptr  */
    }

    return result;
}


template <int inst>
char* __default_alloc_template<inst>::chunk_alloc(size_t bytes, int& nobjs)
{
    size_t total_bytes = bytes * nobjs;
    size_t bytes_left = end_free - start_free;      /* 内存池中剩余的空间 */

    char* result = start_free;
    /* 满足所需 */
    if (bytes_left >= total_bytes) {
        start_free += total_bytes;
        return result;
    }

    /* 不能全满足所需的大小, 但是有一部分可以满足 */
    if (bytes_left >= bytes) {
        nobjs = bytes_left / bytes;
        start_free += (nobjs * bytes);
        return result;
    }

    /* 一个元素都不能分配了, 但内存池中还有一些内存, 
    就将现在内存池中的内存添加到 free_list 中 */
    if (bytes_left > 0) {
        obj** needed_free_list = free_list + FREELIST_INDEX(bytes_left);
        ((obj*)start_free)->free_list_link = *needed_free_list; /* 头插 */
        *needed_free_list = (obj*)start_free;
    }

    /* 一切准备妥当了, 就再申请一大块内存 */
    /* heap_size 会逐渐增大, 不是很理解为何要这样处理 */
    size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);   
    start_free = (char*)malloc(bytes_to_get);
    if (nullptr == start_free) {
        /* 堆上也没有了, 先看看 free_list 上有没有更大的内存区域还没用, 有的化就借来一用 */
        obj** needed_free_list = nullptr; 
        for (size_t i = bytes; i < __MAX_BYTES; i += __ALIGN) {
            needed_free_list = free_list + FREELIST_INDEX(i); 
            
            if (*needed_free_list) {    
                /* 剖离一个节点下来 */
                start_free = (*needed_free_list)->client_data;
                end_free = start_free + i;
                *needed_free_list = (*needed_free_list)->free_list_link;
                return chunk_alloc(bytes, nobjs);   /* 递归的调用自己重新, 调整 nobjs */
            }
        }

        /* 彻底空了, 这时候看一看第一级配置器的 omm 机制能不能起到作用, 
        还是没有就会抛出异常 */
        end_free = nullptr; 
        start_free = (char*)malloc_alloc::allocate(bytes_to_get);
    }

    end_free = start_free + bytes_to_get;
    heap_size += bytes_to_get;              /* heap 每次分配内存都会增加 */
    return chunk_alloc(bytes, nobjs);       /* 取得新内存后, 重新调整 nobjs 的个数 */    
}

__WKANGK_STL_END_NAMESPACE