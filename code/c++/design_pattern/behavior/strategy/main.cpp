/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 策略模式, 采用组合的方式, 将抽象逻辑与具体实现分离
时间	   	: 2023-06-26 19:47
***************************************************************/
#include <iostream>
#include <memory>


/* 这是实现的抽象接口 */
class Strategy
{
public: 
    virtual ~Strategy() = default;

public:
    virtual void AlgorithmInterface() = 0;
};

class ConcreteStrategyA : public Strategy
{
public:
    virtual void AlgorithmInterface()
    {
        std::cout << "ConcreteStrategyA\n";
    }
};

class ConcreteStrategyB : public Strategy
{
public:
    virtual void AlgorithmInterface()
    {
        std::cout << "ConcreteStrategyB\n";
    }
};


/* 而下面这个是逻辑的接口 */
class Context
{
public:
    Context(std::shared_ptr<Strategy> strategy) :
        strategy_(strategy)
    {

    }

public:
    void doAction()
    {   
        /* 具体逻辑调用是实现接口, 这样组合的方式相比继承的方式有一个很好
        的优点是, 可以动态的进行对象策略的替换 */
        strategy_->AlgorithmInterface();
    }

    void setStrategy(std::shared_ptr<Strategy> strategy)
    {
        strategy_ = strategy;
    }

private:
    std::shared_ptr<Strategy> strategy_;
};


int main()
{   
    auto strategyA = std::make_shared<ConcreteStrategyA>();
    auto context = std::make_shared<Context>(strategyA);
    context->doAction();

    auto strategyB = std::make_shared<ConcreteStrategyB>();
    context->setStrategy(strategyB);
    context->doAction();
    return 0;
}