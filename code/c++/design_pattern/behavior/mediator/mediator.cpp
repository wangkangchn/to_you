/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: mediator.hpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 
时间	   	: 2023-07-06 21:38
***************************************************************/
#include "mediator.hpp"
#include "colleague.hpp"


void Mediator::doSomething1()
{
    /* 调用功能类 */
    c1_->self_method();
    c2_->self_method();
}

void Mediator::doSomething2()
{
    c1_->self_method();
    c2_->self_method();
}

