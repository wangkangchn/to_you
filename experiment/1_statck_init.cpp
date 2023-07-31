/***************************************************************
 * @copyright  Copyright © 2023 wkangk.
 * @file       1_statck_init.cpp
 * @author     wkangk <wangkangchn@163.com>
 * @version    v1.0
 * @brief      测试大括号内的静态变量
 * @date       2023-07-30 13:42
 **************************************************************/
#include <iostream>


class A
{
public:
    A() 
    { 
        std::cout << "yes" << std::endl;
    }

    void show() {
        std::cout << "yes" << std::endl;
    }
};

class B
{
public:
    static A a;
    
};
int main()
{
    B b;
    b.a.show();
    return 0;
}