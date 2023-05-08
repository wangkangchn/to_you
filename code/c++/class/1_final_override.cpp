/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: 1_final_override.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 描述	   	: 献给未来的你 - C/C++ --- 类相关知识 1
        (2) final 与 override
时间	   	: 2023-05-08 21:16
***************************************************************/
#include <iostream>


/* -------------------------------------------------------------------------------
 * final
 * ------------------------------------------------------------------------------- */
/* (1) 修饰虚函数 */
class Base
{   
public:
    virtual void foo() final
    {
        std::cout << "Base foo" << std::endl;
    }
};


class Child : public Base
{   
public:
    virtual void foo() 
    {
        std::cout << "Child foo" << std::endl;
    }

    void func() final
    {

    }
};

/* (2) 修饰类 */
class FinalClass final
{

};

class ChildFromFinalClass : public FinalClass
{

};


/* -------------------------------------------------------------------------------
 * override
 * ------------------------------------------------------------------------------- */
class Base1
{
public:
    virtual void foo()
    {
        std::cout << "Base1 foo" << std::endl;
    }
};

class ChildFromBase1 : public Base1
{
public:
    // virtual void foo( int a ) override
    // {
    //     std::cout << "ChildFromBase1 foo" << std::endl;
    // }
    
    virtual void foo() override
    {
        std::cout << "ChildFromBase1 foo" << std::endl;
    }
};