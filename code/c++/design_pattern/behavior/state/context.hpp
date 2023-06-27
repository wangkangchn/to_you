/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: context.hpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 
时间	   	: 2023-06-27 22:21
***************************************************************/
#ifndef __CONTEXT_HPP__ 
#define __CONTEXT_HPP__ 
#include <memory>

class LiftState;
class ClosingState;
class OpenningState;
class RunningState;
class StoppingState;

class Context
{
public:
    static std::shared_ptr<OpenningState> OPENING_STATE;
    static std::shared_ptr<ClosingState> CLOSING_STATE;
    static std::shared_ptr<RunningState> RUNNING_STATE;
    static std::shared_ptr<StoppingState> STOPPING_STATE;

public:
    void setLiftState(LiftState* state);
    LiftState* getLiftState();
    void open();
    void stop();
    void run();
    void close();

private:
    LiftState* current_state_;
};


#endif	/* !__CONTEXT_HPP__ */