/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 访问者模式
    示例参考自 《设计模式之禅》 第 25 章
时间	   	: 2023-07-09 11:52
***************************************************************/

#include "visitor.hpp"
#include "employee.hpp"

void Manager::accept(std::shared_ptr<IVisitor> visitor) const
{
    visitor->visit(this);
}


void CommonEmployee::accept(std::shared_ptr<IVisitor> visitor) const
{
    visitor->visit(this);
}



