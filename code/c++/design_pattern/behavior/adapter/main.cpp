/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 适配器模式
时间	   	: 2023-06-15 20:41
***************************************************************/
#include <iostream>
#include <memory>



class Adaptee
{
public:
    void show() const
    {
        std::cout << "Adaptee" << std::endl;
    }
};

/* 类模式 */
class AdapterClass : private Adaptee
{
public:
    void show() const   /* 名字可以一样 */
    {
        this->Adaptee::show();
    }
};

/* 对象模式 */
class AdapterObject
{
public:
    AdapterObject() : obj(new Adaptee)
    {
    }

    void show_haha() const  /* 也可以不一样, 看自己的需求了 */
    {
        obj->show();
    }

private:
    std::unique_ptr<Adaptee> obj;
};


int main()
{   
    AdapterClass a;
    a.show();

    AdapterClass b;
    b.show();
    return 0;
}