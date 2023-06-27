#include <iostream>

#include "context.hpp"
#include "state.hpp"


/* 当前状态的动作 */
void OpenningState::open() 
{
    std::cout << "电梯门开启 ...\n";
}

/* 当前状态可以转移的动作 */
void OpenningState::close() 
{
    context_->setLiftState(Context::CLOSING_STATE.get());
    context_->getLiftState()->close();
}


void ClosingState::close()
{
    std::cout << "电梯门关闭 ...\n";
}

void ClosingState::open()
{
    context_->setLiftState(Context::OPENING_STATE.get());
    context_->getLiftState()->open();
}

void ClosingState::run()
{
    context_->setLiftState(Context::RUNNING_STATE.get());
    context_->getLiftState()->run();
}

void ClosingState::stop()
{
    context_->setLiftState(Context::STOPPING_STATE.get());
    context_->getLiftState()->stop();
}



/* 这个应该是是本状态对应的动作 */
void RunningState::run()
{
    std::cout << "电梯门上下运行 ...\n";
}

void RunningState::stop()
{
    context_->setLiftState(Context::STOPPING_STATE.get());
    context_->getLiftState()->stop();
}

/* 这个应该是是本状态对应的动作 */
void StoppingState::stop()
{
    std::cout << "电梯门停止了 ...\n";
}

void StoppingState::open()
{
    context_->setLiftState(Context::OPENING_STATE.get());
    context_->getLiftState()->open();
}

void StoppingState::run()
{
    context_->setLiftState(Context::RUNNING_STATE.get());
    context_->getLiftState()->run();
}
