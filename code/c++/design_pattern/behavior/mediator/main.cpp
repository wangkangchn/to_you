/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 
时间	   	: 2023-07-06 21:46
***************************************************************/
#include <memory>

#include "colleague.hpp"
#include "mediator.hpp"


int main()
{
    auto meidator = std::make_shared<Mediator>();
    auto concrete_colleague1 = std::make_shared<ConcreteColleague1>(meidator);
    auto concrete_colleague2 = std::make_shared<ConcreteColleague2>(meidator);
    meidator->setConcreteColleague1(concrete_colleague1);
    meidator->setConcreteColleague2(concrete_colleague2);

    concrete_colleague2->self_method();
    concrete_colleague2->dep_method();
    concrete_colleague1->self_method();
    concrete_colleague1->dep_method();
    return 0;
}