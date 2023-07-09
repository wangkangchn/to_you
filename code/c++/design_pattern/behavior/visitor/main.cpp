/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 
时间	   	: 2023-07-09 12:20
***************************************************************/
#include <iostream>
#include <memory>
#include <vector>

#include "visitor.hpp"
#include "employee.hpp"


int main()
{
    std::shared_ptr<IVisitor> visitor(new ShowVisitor);

    std::vector<std::shared_ptr<IEmployee>> employees;

    auto employee1 = std::make_shared<Manager>("王二麻子", 33, 14002);
    employee1->setPerformance("0000000");
    employees.emplace_back( employee1 );

    auto employee2 = std::make_shared<CommonEmployee>("火哥", 27, 10002);
    employee2->setJob("c++ 程序员");
    employees.emplace_back( employee2 );

    for (auto& employee : employees) {
        employee->accept(visitor);  
        /* 要别的功能, 就可以在这里添加 */
    }

    return 0;
}