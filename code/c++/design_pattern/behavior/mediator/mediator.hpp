/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: mediator.hpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 
时间	   	: 2023-07-06 21:38
***************************************************************/
#ifndef __MEDIATOR_HPP__ 
#define __MEDIATOR_HPP__ 
#include <memory>


class ConcreteColleague1;
class ConcreteColleague2;

/* 中介者 */
class Mediator
{
public:
    void doSomething1();
    void doSomething2();

    void setConcreteColleague1(std::shared_ptr<ConcreteColleague1> c1)
    {
        c1_ = c1;
    }

    void setConcreteColleague2(std::shared_ptr<ConcreteColleague2> c2)
    {
        c2_ = c2;
    }

private:
    /* 中介者要包含功能类, 不然怎么进行交互呀 */
    std::shared_ptr<ConcreteColleague1> c1_;
    std::shared_ptr<ConcreteColleague2> c2_;
};

#endif	/* !__MEDIATOR_HPP__ */