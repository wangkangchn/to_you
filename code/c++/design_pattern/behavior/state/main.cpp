/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 策略模式
    示例参考自 《设计模式之禅》 第 26 章
时间	   	: 2023-06-27 22:02
***************************************************************/
#include "context.hpp"
#include "state.hpp"


int main()
{
    Context context;

    context.setLiftState(Context::CLOSING_STATE.get());
    context.open(); /* 会由上一状态转移到下一状态!!! 然后再执行, 最后停留在想要的状态中!!! */
    context.close();
    context.run();
    context.stop();


    return 0;
}



