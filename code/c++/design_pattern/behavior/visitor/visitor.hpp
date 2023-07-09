/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: visitor.hpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 访问者
时间	   	: 2023-07-09 11:57
***************************************************************/
#ifndef __VISITOR_HPP__ 
#define __VISITOR_HPP__ 
#include <memory>

#include "employee.hpp"



/* 观察者的抽象基类 */
class IVisitor
{
public:
    virtual ~IVisitor() = default;

public:
    /* 访问者需要依赖具体的子类, 通过重载来来实现不同
    对象的调用 */
    virtual void visit(const Manager* employee) const = 0;
    virtual void visit(const CommonEmployee* employee) const = 0;
};


class ShowVisitor : public IVisitor
{
public:
    virtual void visit(const Manager* employee) const override
    {
        std::string info = getBaseInfo(employee);
        info += ", 业绩: " + employee->getPerformance();
        std::cout << info << std::endl;
    }

    /* 米格访问者应该对业务员完全了解!!! */
    virtual void visit(const CommonEmployee* employee) const override
    {
        std::string info = getBaseInfo(employee);
        info += ", 工作: " + employee->getJob();
        std::cout << info << std::endl;
    }

private:
    std::string getBaseInfo(const IEmployee* employee) const
    {
        std::string info = "";
        info += "姓名: " + employee->getName() + ", ";
        info += "年龄: " + std::to_string( employee->getAge()) + ", ";
        info += "薪水: " + std::to_string( employee->getSalary());
        return info;
    }
};
#endif	/* !__VISITOR_HPP__ */