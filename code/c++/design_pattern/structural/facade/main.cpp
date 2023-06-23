/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 门面模式
时间	   	: 2023-06-20 21:16
***************************************************************/
#include <iostream>
#include <memory>


class SubSystem1
{
public:
    void show() const 
    {
        std::cout << "SubSystem1" << std::endl;
    }
};

class SubSystem2
{
public:
    void haha() const 
    {
        std::cout << "SubSystem2" << std::endl;
    }
};


/* 这个就是为 main 模块提供操作的门面类
门面类应该只进行转发, 不进行业务逻辑的处理
以前一直觉得这种转发是不是很浪费啊, 现在看来
还是我太年轻了啊, 哈哈哈 */
class System
{
public:
    System() :
        sub_system1(new SubSystem1),
        sub_system2(new SubSystem2)
    {
    }

public:
    void sub_system1_operate() 
    {
        sub_system1->show();
    }

    void sub_system2_operate() 
    {
        sub_system2->haha();
    }

private:
    std::shared_ptr<SubSystem1> sub_system1;
    std::shared_ptr<SubSystem2> sub_system2;
};


int main()
{   
    auto system = std::make_shared<System>();
    system->sub_system1_operate();
    system->sub_system2_operate();
    return 0;
}