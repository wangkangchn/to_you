/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 中介者
    示例参考自 《设计模式之禅》 第 14 章
时间	   	: 2023-07-06 21:25
***************************************************************/
#include <iostream>

#include "colleague.hpp"
#include "mediator.hpp"


void ConcreteColleague1::self_method()
{
    std::cout << "ConcreteColleague1::self_method 自己的方法\n";
}

void ConcreteColleague1::dep_method()
{
    std::cout << "ConcreteColleague1::dep_method 依赖的方法\n";
    mediator_->doSomething1();
}


void ConcreteColleague2::self_method()
{
    std::cout << "ConcreteColleague2::self_method 自己的方法\n";
}

void ConcreteColleague2::dep_method()
{
    std::cout << "ConcreteColleague2::dep_method 依赖的方法\n";
    mediator_->doSomething2();
}