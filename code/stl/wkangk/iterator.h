/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: iterator.h
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 简易迭代器
时间	   	: 2023-05-19 20:23
***************************************************************/
#ifndef __WKANGK_STL_ITERATOR_H__ 
#define __WKANGK_STL_ITERATOR_H__ 
#include <stddef.h>

#include "config.h"

__WKANGK_STL_BEGIN_NAMESPACE


/* -------------------------------------------------------------------------------
 * 五种迭代器类型
 * ------------------------------------------------------------------------------- */
struct input_iterator_tag {};   /* 只读 */
struct output_iterator_tag {};  /* 唯写 */
struct forward_iterator_tag : public input_iterator_tag {};  /* 可读可写, 只可前向移动 */
struct bidirectional_iterator_tag : public forward_iterator_tag {};  /* 可读可写, 可双向移动 */
struct random_access_iterator_tag : public bidirectional_iterator_tag {};  /* 可读可写, 可随机移动 */

/* 输入迭代器, 只允许读. 只要是迭代器都需要定义五个类型, 方便萃取, 
满足 STL 的要求 */
template <typename T, typename Distamce>
struct input_iterator
{   
    typedef input_iterator_tag  iterator_category;      /* 迭代器类型 */
    typedef T                   value_type;             /* 迭代器所指对象的类型 */
    typedef Distamce            difference_type;        /* 两个迭代器间距离类型 */
    typedef T*                  pointer;                /* 迭代器所指对象的指针类型 */
    typedef T&                  reference;              /* 迭代器所指对象的引用类型  */
};

/* 输出迭代器, 只允许写, 不允许读, 所以迭代器所指对象的类型不应为外界获取 */
template <typename T, typename Distamce>
struct output_iterator
{   
    typedef output_iterator_tag iterator_category;      /* 迭代器类型 */
    typedef void                value_type;             /* 迭代器所指对象的类型 */
    typedef void                difference_type;        /* 两个迭代器间距离类型 */
    typedef void                pointer;                /* 迭代器所指对象的指针类型 */
    typedef void                reference;              /* 迭代器所指对象的引用类型  */
};

/* 前向迭代器 应提供 ++ */
template <typename T, typename Distamce>
struct forward_iterator
{   
    typedef forward_iterator_tag iterator_category;      /* 迭代器类型 */
    typedef T                   value_type;             /* 迭代器所指对象的类型 */
    typedef Distamce            difference_type;        /* 两个迭代器间距离类型 */
    typedef T*                  pointer;                /* 迭代器所指对象的指针类型 */
    typedef T&                  reference;              /* 迭代器所指对象的引用类型  */
};

/* 双向迭代器 应提供 ++, -- */
template <typename T, typename Distamce>
struct bidirectional_iterator
{   
    typedef bidirectional_iterator_tag  iterator_category;      /* 迭代器类型 */
    typedef T                           value_type;             /* 迭代器所指对象的类型 */
    typedef Distamce                    difference_type;        /* 两个迭代器间距离类型 */
    typedef T*                          pointer;                /* 迭代器所指对象的指针类型 */
    typedef T&                          reference;              /* 迭代器所指对象的引用类型  */
};

/* 随机迭代器 应提供 ++, --, [] */
template <typename T, typename Distamce>
struct random_access_iterator
{   
    typedef random_access_iterator_tag  iterator_category;      /* 迭代器类型 */
    typedef T                           value_type;             /* 迭代器所指对象的类型 */
    typedef Distamce                    difference_type;        /* 两个迭代器间距离类型 */
    typedef T*                          pointer;                /* 迭代器所指对象的指针类型 */
    typedef T&                          reference;              /* 迭代器所指对象的引用类型  */
};


/* 为方便扩展扩展, STL 为我们创建了一个迭代器基类 */
template <typename Iterator,
          typename Value, 
          typename Distance  = ptrdiff_t,
          typename Pointer   = Value*,
          typename Reference = Value&>
struct iterator
{
    typedef Iterator    iterator_category;
    typedef Value       value_type;        
    typedef Iterator    difference_type;   
    typedef Pointer     pointer;           
    typedef Reference   reference;         
};


/* -------------------------------------------------------------------------------
 * 迭代器 traits
 * ------------------------------------------------------------------------------- */
/* 可以方便的获取迭代器内定义的五种类型, 
为什么不直接使用迭代器类型由用户来获取呢?
为了使原生指针也可以使用与迭代器通用的方式来获取各种类型, 对, 就是这样

就可以通过 
iterator_traits<Iterator>::value_type   来获取对象类型
iterator_traits<int*>::value_type       来获取对象类型
 */
template <typename Iterator>
struct iterator_traits
{
    typedef typename Iterator::iterator_category    iterator_category;
    typedef typename Iterator::value_type           value_type;
    typedef typename Iterator::difference_type      difference_type;   
    typedef typename Iterator::pointer              pointer;           
    typedef typename Iterator::reference            reference;         
};

/* 原生指针是天生的随机迭代器, 但是其并没有定义我们所需的五种类型, 
所以为了方便使用, 要对其进行偏特化处理 */
template <typename T>
struct iterator_traits<T*>
{
    typedef random_access_iterator_tag              iterator_category;
    typedef T                                       value_type;
    typedef ptrdiff_t                               difference_type;   
    typedef T*                                      pointer;           
    typedef T&                                      reference;         
};

template <typename T>
struct iterator_traits<const T*>
{
    typedef random_access_iterator_tag              iterator_category;
    typedef T                                       value_type;
    typedef ptrdiff_t                               difference_type;   
    typedef const T*                                pointer;           
    typedef const T&                                reference;         
};

/* 为什么不为 & 设计偏特化?
因为引用不是迭代器 */


/* 各类型的方便萃取函数,
这里关于 inline 要多说一点. 一般大家都说 inline 是通知编译器将这个函数做内联
处理, 但是实际上编译器做不做内联并不会看 inline 标记, 只要它认为这个函数足够
短小就会进行内联, 无论有没有 inline 标记. 反之就算有 inline 标记, 但是这个函
数足够大, 编译器也不会进行内联.
inline 真正的作用是打破编译器设置的单定义规则. 编译器对变量/函数的定义有一个
单定义规则: 也就是定义只能出现一次. 出现两次就会出现多次定义的编译错误.
而如果我们要将函数体写在头文件中时, 因为头文件会被别的文件多次包含, 就会导致
其中的函数体出现多次, 编译器就会认为这个函数被定义了多次(就算是使用了 #ifdef
也不行, 不能通过编译, 猜测可能是编译器会先全部替换, 再选择编译). 
而在头文件中声明 inline 时, 就是告知编译器消除单定义原则, 可以允许不同的文件
中有多个实现, 但是要求所有的实现必须相同! 
 */
template <typename Iterator>
inline typename iterator_traits<Iterator>::iterator_category
iterator_category(const Iterator&)
{   
    /* 编译器无法区分 iterator_category 到底是类内的成员还是类型, 
    默认会编译器会认为是成员. 所以当使用类型的时候要使用 typename
    显式告诉编译器  */
    return typename iterator_traits<Iterator>::iterator_category();
}

/* 为啥要返回指针?? 没理解 */
template <typename Iterator>
typename iterator_traits<Iterator>::value_type*
value_type(const Iterator&)
{   
    typedef typename iterator_traits<Iterator>::value_type value_type;
    return static_cast<value_type*>(0);
}

template <typename Iterator>
typename iterator_traits<Iterator>::difference_type*
difference_type(const Iterator&)
{   
    typedef typename iterator_traits<Iterator>::difference_type difference_type;
    return static_cast<difference_type*>(0);
}


/* 由为各个迭代器提供了重载, 感觉没多少必要, 毕竟已经有了通用的 iterator_category */
template <typename T, typename Distance>
inline input_iterator_tag
iterator_category(const input_iterator<T, Distance>&)
{
    return input_iterator_tag();
}

template <typename T, typename Distance>
inline output_iterator_tag
iterator_category(const output_iterator<T, Distance>&)
{
    return output_iterator_tag();
}

template <typename T, typename Distance>
inline forward_iterator_tag
iterator_category(const forward_iterator<T, Distance>&)
{
    return forward_iterator_tag();
}

template <typename T, typename Distance>
inline bidirectional_iterator_tag
iterator_category(const bidirectional_iterator<T, Distance>&)
{
    return bidirectional_iterator_tag();
}

template <typename T, typename Distance>
inline random_access_iterator_tag
iterator_category(const random_access_iterator<T, Distance>&)
{
    return random_access_iterator_tag();
}

template <typename T>
inline random_access_iterator_tag
iterator_category(const T*)
{
    return random_access_iterator_tag();
}

template <typename T>
inline random_access_iterator_tag
iterator_category(T*)
{
    return random_access_iterator_tag();
}

/* value_type */
template <typename T, typename Distance>
inline T* value_type(const input_iterator<T, Distance>&)
{
    return (T*)(0);
}
template <typename T, typename Distance>
inline T* value_type(const output_iterator<T, Distance>&)
{
    return (T*)(0);
}
template <typename T, typename Distance>
inline T* value_type(const forward_iterator<T, Distance>&)
{
    return (T*)(0);
}
template <typename T, typename Distance>
inline T* value_type(const bidirectional_iterator<T, Distance>&)
{
    return (T*)(0);
}
template <typename T, typename Distance>
inline T* value_type(const random_access_iterator<T, Distance>&)
{
    return (T*)(0);
}
template <typename T>
inline T* value_type(const T*)
{
    return (T*)(0);
}


/* value_type */
template <typename T, typename Distance>
inline Distance* difference_type(const input_iterator<T, Distance>&)
{
    return (Distance*)(0);
}
template <typename T, typename Distance>
inline Distance* difference_type(const output_iterator<T, Distance>&)
{
    return (Distance*)(0);
}
template <typename T, typename Distance>
inline Distance* difference_type(const forward_iterator<T, Distance>&)
{
    return (Distance*)(0);
}
template <typename T, typename Distance>
inline Distance* difference_type(const bidirectional_iterator<T, Distance>&)
{
    return (Distance*)(0);
}
template <typename T, typename Distance>
inline Distance* difference_type(const random_access_iterator<T, Distance>&)
{
    return (Distance*)(0);
}
template <typename T>
inline ptrdiff_t* difference_type(const T*)
{
    return (ptrdiff_t*)(0);
}


/* -------------------------------------------------------------------------------
 * distance 相关
 * ------------------------------------------------------------------------------- */
/* 因为参数类型都一样, 为了使重载生效, 我们就需要使用不同的参数表, 这就是
tag 的作用 */
/* 计算两个迭代器间的距离 */
template <typename InputIterator, class Distance>
inline void __distance(InputIterator first, InputIterator last, Distance& n, input_iterator_tag)
{
    while (first != last) {
        ++first;
        ++n;
    }
}

template <typename RandomAccessIterator, class Distance>
inline void __distance(RandomAccessIterator first, RandomAccessIterator last, Distance& n, random_access_iterator_tag)
{
    n += last - first;
}

/* 为什么使用 InputIterator, 而不是使用一个泛型名字, 毕竟 distance
是任何一个迭代器都可以使用的. 书中提到过这是为了遵循 STL 算法的命名规则:
以算法能接受之最初级来为其迭代器型别参数命名.
命名为 InputIterator 就是告诉使用者, 这个方法接受任何迭代器类型
传入随机迭代器就会调用随机迭代器版本的距离方法 */
template <typename InputIterator, class Distance>
inline void distance(InputIterator first, InputIterator last, Distance& n)
{
    __distance(first, last, n, iterator_category(first));
}

/* 还有偏特化版本 */
template <typename InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
__distance(InputIterator first, InputIterator last, input_iterator_tag)
{
    typename iterator_traits<InputIterator>::difference_type n = 0;
    while (first != last) {
        ++first;
        ++n;
    }
    return n;
}

template <typename RandomAccessIterator>
inline typename iterator_traits<RandomAccessIterator>::difference_type
__distance(RandomAccessIterator first, RandomAccessIterator last, random_access_iterator_tag)
{
    return last - first;
}

/* 为什么只实现 input 和 random ? 因为除了 random 外的所有迭代器都可以使用同一种方式进行计算 */
template <typename InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
distance(InputIterator first, InputIterator last)
{   
    return __distance(first, last, iterator_category(first));
}


/* -------------------------------------------------------------------------------
 * advance 相关
 * ------------------------------------------------------------------------------- */
/* tag 没有参数. 仅仅是为了触发 c++ 的函数重载机制, 所以不需要参数只要类型就好
重载只看参数列表, 不看返回值!!! */
/* 将迭代器前进 n 步 */
template <typename InputIterator, class Distance>
inline void __advance(InputIterator& iter, Distance n, input_iterator_tag)
{
    while (n--) {
        ++iter;
    }
}

template <typename BidirectionalIterator, class Distance>
inline void __advance(BidirectionalIterator& iter, Distance n, bidirectional_iterator_tag)
{
    if (n >= 0) {
        while (n--) {
            ++iter;
        }
    } else {
        while (n++) {
            --iter;
        }
    }
}

template <typename RandomAccessIterator, class Distance>
inline void __advance(RandomAccessIterator& iter, Distance n, random_access_iterator_tag)
{
    iter += n;
}

template <typename InputIterator, class Distance>
inline void advance(InputIterator& iter, Distance n)
{
    return __advance(iter, n, iterator_category(iter));
}



/* -------------------------------------------------------------------------------
 * 具体使用的迭代器
 * ------------------------------------------------------------------------------- */
template <typename Container>
class back_insert_iterator
{
protected:
    Container* container;           /* 迭代器应用的容器 */

public:
    typedef output_iterator_tag iterator_category;  /* 输出, 唯写, 输出到容器 */
    typedef void                value_type;
    typedef void                difference_type;
    typedef void                pointer;
    typedef void                reference;

    explicit back_insert_iterator(Container& x) :
        container(&x)
    {}
    
    /* 迭代器就是一个广义指针, 所以可以直接将值保存到其中 */
    back_insert_iterator&
    operator=(const typename Container::value_type& value) 
    {
        container->push_back(value);
        return *this;
    }

    back_insert_iterator<Container>& operator*() { return *this; }
    back_insert_iterator<Container>& operator++() { return *this; }     /* 前++ */
    back_insert_iterator<Container>& operator++(int) { return *this; }  /* 后++ */
};

/* 偏特化, 获取迭代器类型, 可能更快 */
template <typename Container>
output_iterator_tag
iterator_category(const back_insert_iterator<Container>&)
{
    return output_iterator_tag();
}

/* 为指定的容器构造一个尾插迭代器 */
template <typename Container>
back_insert_iterator<Container> back_inserter(Container& container)
{
    return back_insert_iterator<Container>(container);
}


/* 头插迭代器 */
template <typename Container>
class front_insert_iterator
{
protected:
    Container* container;           /* 迭代器应用的容器 */

public:
    typedef output_iterator_tag iterator_category;  /* 输出, 唯写, 输出到容器 */
    typedef void                value_type;
    typedef void                difference_type;
    typedef void                pointer;
    typedef void                reference;

    explicit front_insert_iterator(Container& x) :
        container(&x)
    {}
    
    /* 迭代器就是一个广义指针, 所以可以直接将值保存到其中 */
    front_insert_iterator&
    operator=(const typename Container::value_type& value) 
    {
        container->push_front(value);               /* 模板更像是一种规范, 只有满足
                                                    这种规范才可以使用 */
        return *this;
    }

    front_insert_iterator<Container>& operator*() { return *this; }
    front_insert_iterator<Container>& operator++() { return *this; }     /* 前++ */
    front_insert_iterator<Container>& operator++(int) { return *this; }  /* 后++ */
};

/* 偏特化, 获取迭代器类型, 可能更快 */
template <typename Container>
output_iterator_tag
iterator_category(const front_insert_iterator<Container>&)
{
    return output_iterator_tag();
}

/* 为指定的容器构造一个尾插迭代器 */
template <typename Container>
front_insert_iterator<Container> front_inserter(Container& container)
{
    return front_insert_iterator<Container>(container);
}


/* 插入迭代器 */
template <typename Container>
class insert_iterator
{
protected:
    Container* container;               /* 迭代器应用的容器 */
    typename Container::iterator iter;  /* 具体的迭代器应由容器来指定 */

public:
    typedef output_iterator_tag iterator_category;  /* 输出, 唯写, 输出到容器 */
    typedef void                value_type;
    typedef void                difference_type;
    typedef void                pointer;
    typedef void                reference;

    explicit insert_iterator(Container& x, typename Container::iterator& iter) :
        container(&x), iter(iter)
    {}
    
    /* 迭代器就是一个广义指针, 所以可以直接将值保存到其中 */
    insert_iterator&
    operator=(const typename Container::value_type& value) 
    {
        iter = container->insert(iter, value);          /* 在迭代器指定的位置上插入新值 */
        ++iter;
        return *this;
    }

    insert_iterator<Container>& operator*() { return *this; }
    insert_iterator<Container>& operator++() { return *this; }     /* 前++ */
    insert_iterator<Container>& operator++(int) { return *this; }  /* 后++ */
};

/* 偏特化, 获取迭代器类型, 可能更快 */
template <typename Container>
output_iterator_tag
iterator_category(const insert_iterator<Container>&)
{
    return output_iterator_tag();
}

/* 为指定的容器构造一个尾插迭代器 */
template <typename Container, typename Iterator>
insert_iterator<Container> inserter(Container& container, Iterator iter)
{   
    /* 运行迭代器强转 */
    return inserter<Container>(container, typename Container::iterator(iter));
}


/* 反向迭代器 */
template <typename BidirectionalIterator, typename T,
            typename Reference=T&,
            typename Distance=ptrdiff_t>
class reverse_bidirectional_iterator
{
    typedef reverse_bidirectional_iterator<BidirectionalIterator, T, Reference, Distance> self;

protected:
    BidirectionalIterator current;  /* 传入的参数和这个类是什么关系, 
                                    又封装一层, 将所有同类操作封装在一起 */

public:
    typedef bidirectional_iterator_tag  iterator_category;  /* 输出, 唯写, 输出到容器 */
    typedef T                           value_type;
    typedef Distance                    difference_type;
    typedef T*                          pointer;
    typedef T&                          reference;

    reverse_bidirectional_iterator() = default;
    explicit reverse_bidirectional_iterator(BidirectionalIterator iter) :
        current(iter)
    {}

    BidirectionalIterator base() const { return current; }

    Reference operator*() const 
    { 
        /* 因为这个是反向迭代器, 所以 current 指出的是容器最后一个元素的下一个位置,
        所以解引的时候必须向前移动, 才能得到值 */
        BidirectionalIterator tmp = current;
        return *--tmp; 
    }

    pointer operator->() const
    {
        return &(operator*());  /* 因为返回的是引用, 所以这里可以取址 */
    }

    self& operator++()
    {
        --current;              /* 因为是反向迭代器, 所以 ++ 的使用, 应该向前走 */
        return *this;
    }

    self operator++(int)
    {
        self tmp = *this;
        --current;
        return tmp;         /* 后++, 在操作的同时, 返回旧值 */
    }

    self& operator--()
    {
        ++current;
        return *this;
    }

    self operator--(int)
    {
        self tmp = *this;
        ++current;
        return tmp;         /* 后++, 在操作的同时, 返回旧值 */
    }
};

template <class BidirectionalIterator, class T, class Reference, 
          class Distance>
bidirectional_iterator_tag
iterator_category(const reverse_bidirectional_iterator<BidirectionalIterator, T,
                            Reference, Distance>&)
{
    return bidirectional_iterator_tag();
}

template <class BidirectionalIterator, class T, class Reference, 
          class Distance>
T*
value_type(const reverse_bidirectional_iterator<BidirectionalIterator, T,
                            Reference, Distance>&)
{
    return (T*)(0);
}

template <class BidirectionalIterator, class T, class Reference, 
          class Distance>
Distance*
distance_type(const reverse_bidirectional_iterator<BidirectionalIterator, T,
                            Reference, Distance>&)
{
    return (Distance*)(0);
}

/* 判断两个迭代器指针是不是一样 */
template <class BidirectionalIterator, class T, class Reference, 
          class Distance>
bool operator==(const reverse_bidirectional_iterator<BidirectionalIterator, T,
                            Reference, Distance>& lhs,
                const reverse_bidirectional_iterator<BidirectionalIterator, T,
                            Reference, Distance>& rhs)
{
    return lhs.base() == rhs.base();
}

__WKANGK_STL_END_NAMESPACE

#endif	/* !__WKANGK_STL_ITERATOR_H__ */