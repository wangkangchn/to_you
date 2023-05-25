/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: 2_pod.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 测试类型是否为 POD 的
时间	   	: 2023-05-23 20:48
***************************************************************/
#include <type_traits>
#include <iostream>
#include <memory>

class A
{
public:
    A() = default;
    // A(int a) : a_(a) {}
    ~A() {};
    
private:
    // std::shared_ptr<int> b_;
    int a_;
};


int main()
{
    std::cout << std::is_pod<A>::value << std::endl;
    return 0;
}