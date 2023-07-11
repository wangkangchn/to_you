/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 迭代器模式
时间	   	: 2023-07-11 20:38
***************************************************************/
#include <vector>
#include <iostream>


int main()
{
    std::vector<int> vec{1,2,3,4,5,56,6,7,7,8,8,9,9,0,0};

    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << std::endl;
    }
    return 0;
}