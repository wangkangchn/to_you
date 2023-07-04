/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 备忘录模式
    实例参考自 《设计模式之禅》 第 24 章
时间	   	: 2023-07-04 21:35
***************************************************************/
#include <iostream>
#include <memory>
#include <string>


class Memento
{
public:
    Memento(const std::string& state) : 
        state_(state)
    {
    }

public:
    void setState(const std::string& state) 
    { 
        state_ = state;
    }

    std::string getState() const 
    { 
        return state_; 
    }

private:
    std::string state_;
};

class Caretaker
{
public:
    std::shared_ptr<Memento> getMemento() const
    {
        return memento_;
    }

    void setMemento(std::shared_ptr<Memento> memento)
    {
        memento_ = memento;
    }

private:
    std::shared_ptr<Memento> memento_;
};

/* 备忘录模式可以有很多种不同的实现方式, 要看具体情况的选择了,
这里只是其中的一种, 使用外部的管理器来管理备忘录 */
class Originator
{
public:
    std::string getState() const
    {
        return state_;
    }

    void setState(const std::string& state)
    {
        state_ = state;
    }

    std::shared_ptr<Memento> createMemento()
    {
        return std::make_shared<Memento>(this->state_);
    }

    void restoreMemento(std::shared_ptr<Memento> memento)
    {
        setState(memento->getState());
    }

private:
    std::string state_;
};


int main()
{
    auto origiator = std::make_shared<Originator>();
    origiator->setState("状态 1");
    std::cout << "初始状态为: " << origiator->getState() << std::endl;

    /* 备份管理器 */
    auto caretaker = std::make_shared<Caretaker>();

    /* 创建一个备份交由管理器进行管理 */
    caretaker->setMemento(origiator->createMemento());


    /* 更改状态 */
    origiator->setState("状态 2");
    std::cout << "更改后状态为: " << origiator->getState() << std::endl;

    /* 回退 */
    origiator->restoreMemento(caretaker->getMemento());
    std::cout << "回退后状态为: " << origiator->getState() << std::endl;


    return 0;
}