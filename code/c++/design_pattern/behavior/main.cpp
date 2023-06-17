/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 装饰器模式
时间	   	: 2023-06-17 11:58
***************************************************************/
#include <iostream>
#include <memory>
#include <chrono>

/* 具体功能的基类, 提供操作接口 */
class Component
{
public:
    virtual void operate() = 0;
};


/* 具体功能类 */
class ConcreteComponent : public Component
{
public:
    virtual void operate() 
    {
        std::cout << "我是 ConcreteComponent, 我开始干活了\n";
    }
};


/* 装饰器接口, 继承功能类, 提供一致的接口, 这个基类主要是为了
设置成员方便后续调用 */
class Decorator : public Component
{
public:
    Decorator(Component* component) :
        component(component)
    {
    }

    virtual void operate() 
    {
        component->operate();
    }

private:
    Component* component;   /* 可以不考虑释放问题, 由客端控制 */
};


    
/* 统计时间 */
class TimeConsumptionDecorator : public Decorator
{
public:
    using Decorator::Decorator;

    virtual void operate() 
    {
        std::cout << "我是计时装饰器, 我开始干活了\n";
        auto t1 = std::chrono::steady_clock::now();
        this->Decorator::operate();
        auto t2 = std::chrono::steady_clock::now();

        auto time_used = std::chrono::duration_cast<std::chrono::duration<double>> (t2 - t1);
        std::cout << "功能耗时: " << time_used.count() * 1000 << " ms" << std::endl;
    }
};

class ConcreteDecorator : public Decorator
{
public:
    using Decorator::Decorator;

    virtual void operate() 
    {
        std::cout << "我是 ConcreteDecorator, 我开始干活了\n";
        this->Decorator::operate();
    }
};


int main()
{
    Component* component = new ConcreteComponent;

    component = new TimeConsumptionDecorator(component);
    component = new ConcreteDecorator(component);
    component->operate();
    return 0;
}