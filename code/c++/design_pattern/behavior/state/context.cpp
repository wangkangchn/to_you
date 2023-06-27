/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: context.hpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 
时间	   	: 2023-06-27 22:21
***************************************************************/
#include "context.hpp"
#include "state.hpp"


void Context::setLiftState(LiftState* state)
{
    current_state_ = state;
    current_state_->setContext(this);   /* 设置状态的上下文 */
}

LiftState* Context::getLiftState()
{
    return current_state_;
}

void Context::open()
{
    current_state_->open();
}

void Context::stop()
{
    current_state_->stop();
}

void Context::run()
{
    current_state_->run();
}

void Context::close()
{
    current_state_->close();
}


std::shared_ptr<OpenningState> Context::OPENING_STATE = std::make_shared<OpenningState>();
std::shared_ptr<ClosingState> Context::CLOSING_STATE = std::make_shared<ClosingState>();
std::shared_ptr<RunningState> Context::RUNNING_STATE = std::make_shared<RunningState>();
std::shared_ptr<StoppingState> Context::STOPPING_STATE = std::make_shared<StoppingState>();