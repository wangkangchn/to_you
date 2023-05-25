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
    第一级内存分配器, 分配大于 128K 的数据
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


__WKANGK_STL_END_NAMESPACE