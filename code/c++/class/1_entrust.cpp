/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: 1.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 献给未来的你 - C/C++ --- 类相关知识 1
        (1) 委托构造
时间	   	: 2023-05-08 20:44
***************************************************************/
#include <iostream>

/* -------------------------------------------------------------------------------
 * 1. 委托构造
 * ------------------------------------------------------------------------------- */
/* (1) 使用自身已定义的构造函数 */
class A
{
public:
    A(int a) : 
        a_(a), b_(0), c_(0) 
    {}
    
    /* 委托构造本类时, 初始化列表中只允许有出现委托的构造函数,
    不允许出现其余变量的初始化 */
    // A(int a, int b, int c) :
    //     A(a), b_(b), c_(c)
    // {}
    
    A(int a, int b, int c) :
        A(a)
    {
        b_ = b;
        c_ = c;
    }

    void show() const
    {
        std::cout << "a_: " << a_ << std::endl;
        std::cout << "b_: " << b_ << std::endl;
        std::cout << "c_: " << c_ << std::endl;
    }

private:
    int a_;
    int b_;
    int c_;
};


/* (2) 使用父类定义的构造函数 */
class Base
{   
public:
    Base(int a) :
        a_(a)
    {}

private:
    int a_;
};

/* 不使用委托构造的方式, 子类中父类构造不可见 */
// class Child : public Base
// {   
// };

class Child : public Base
{   
public:
    using Base::Base;
};



int main()
{   
    A a(1, 2, 3);
    a.show();

    Child child(1);
    return 0;
}