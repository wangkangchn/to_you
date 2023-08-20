/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: turtle.hpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: mock 要模拟的基类
时间	   	: 2023-07-16 08:41
***************************************************************/
#ifndef __TURTLE_HPP__ 
#define __TURTLE_HPP__ 

class Data
{
public:
    Data(int a, int b) : a_(a), b_(b) {}

public:
    int a_;
    int b_;
};

class Turtle 
{
public:
    virtual ~Turtle() = default;
    virtual void PenUp() = 0;
    virtual void PenDown() = 0;
    virtual void Forward(int distance) = 0;
    virtual void Turn(int degrees) = 0;
    virtual void GoTo(int x, int y) = 0;
    virtual int GetX() const = 0;
    virtual int GetY() const = 0;
    virtual Data GetData() const = 0;
};
#endif	/* !__TURTLE_HPP__ */