/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 访问者模式
    示例参考自 《设计模式之禅》 第 25 章
时间	   	: 2023-07-09 11:52
***************************************************************/
#ifndef __EMPLOYEE_HPP__ 
#define __EMPLOYEE_HPP__ 
#include <memory>
#include <string>
#include <iostream>

#include "visitor.hpp"

class IVisitor;

/* 员工的抽象基类 */
class IEmployee
{
public:
    IEmployee(const std::string& name, int age, int salary) :
        name_(name), age_(age), salary_(salary)
    {
    }
    
    virtual ~IEmployee() = default;

public:
    /* 接受任意符合的访问者访问 */
    virtual void accept(std::shared_ptr<IVisitor> visitor) const = 0;

    int getSalary() const { return salary_; }
    int getAge() const { return age_; }
    std::string getName() const { return name_; }

    void setName(std::string name) { name_ = std::move(name); }
    void setAge(int age) { age_ = age; }
    void setSalary(int salary) { salary_ = salary; }

private:
    int salary_;    
    int age_;
    std::string name_;
};


class Manager : public IEmployee
{
public:
    using IEmployee::IEmployee;

public: 
    virtual void accept(std::shared_ptr<IVisitor> visitor) const override;

    std::string getPerformance() const 
    {
        return performance_;
    }

    void setPerformance(std::string performance) 
    {
        performance_ = std::move(performance);
    }

private:
    std::string performance_;
};

class CommonEmployee : public IEmployee
{
public:
    using IEmployee::IEmployee;

public: 
    virtual void accept(std::shared_ptr<IVisitor> visitor) const override;

    std::string getJob() const 
    {
        return job_;
    }

    void setJob(std::string job) 
    {
        job_ = std::move(job);
    }

private:
    std::string job_;
};

#endif	/* !__EMPLOYEE_HPP__ */


