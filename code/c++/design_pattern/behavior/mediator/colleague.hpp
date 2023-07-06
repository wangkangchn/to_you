/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 中介者
    示例参考自 《设计模式之禅》 第 14 章
时间	   	: 2023-07-06 21:25
***************************************************************/
#ifndef __COLLEAGUE_HPP__ 
#define __COLLEAGUE_HPP__ 
#include <iostream>
#include <memory>

class Mediator;

/* 具体业务类 */
class Colleague
{
public:
    virtual ~Colleague() = default;

    /* 所有的功能类都需要维持一个中介者, 但需要与其它类进行交互时, 
    就通过中介者进行 */
    Colleague(std::shared_ptr<Mediator> mediator) : 
        mediator_(mediator)
    {
    }

protected:
    std::shared_ptr<Mediator> mediator_;
};


class ConcreteColleague1 : public Colleague
{
public:
    using Colleague::Colleague;

public:
    void self_method();
    void dep_method();
};


class ConcreteColleague2 : public Colleague
{
public:
    using Colleague::Colleague;

public:
    void self_method();
    void dep_method();
};

#endif	/* !__COLLEAGUE_HPP__ */