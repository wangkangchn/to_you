/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 原型模式
时间	   	: 2023-06-12 20:50
***************************************************************/
#include <iostream>
#include <memory>


class Prototype
{
public:
    virtual ~Prototype() = default;
    virtual std::shared_ptr<Prototype> clone() = 0;
};

class ConcretePrototype : public Prototype
{
public:
    virtual std::shared_ptr<Prototype> clone()
    {   
        /* 借助拷贝函数实现 */
        /* 需要有特殊使用场景时, 可以构建 clone, 否则我觉得直接用拷贝构造就行
        可能是我看的太少, 等看的多了, 可能就有更多想法了 */
        return std::make_shared<ConcretePrototype>(*this);
    }
};


int main()
{   
    ConcretePrototype prototype;
    prototype.clone();
    return 0;
}