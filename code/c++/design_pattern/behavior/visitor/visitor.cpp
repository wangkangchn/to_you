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


class Manager;
class CommonEmployee;



/* 观察者的抽象基类 */
class IVisitor
{
public:
    virtual ~IVisitor() = default;

public:
    /* 访问者需要依赖具体的子类, 通过重载来来实现不同
    对象的调用 */
    virtual void visit(Manager* employee) = 0;
    virtual void visit(CommonEmployee* employee) = 0;
};

#endif	/* !__VISITOR_HPP__ */