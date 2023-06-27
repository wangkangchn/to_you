/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: state.hpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 
时间	   	: 2023-06-27 22:22
***************************************************************/
#ifndef __STATE_HPP__ 
#define __STATE_HPP__ 

class Context;

/**
 *     电梯状态
 */
class LiftState
{
public:
    virtual ~LiftState() = default;

public:
    /* 默认什么都不做 */
    virtual void open() {}
    virtual void close() {}
    virtual void run() {}
    virtual void stop() {}

    /* 设置当前状态所述的上下文, 那这也可以看出来, 同样的状态对象
    也可以在不同的上下文中进行使用! 秒啊 */
    void setContext(Context* context)
    {
        context_ = context;
    }

protected:
    Context* context_;      /* 状态切换的上下文 */
};

/* 开门状态 */
class OpenningState : public LiftState
{
public:
    /* 当前状态的动作 */
    virtual void open() override;

    /* 当前状态可以转移的动作 */
    virtual void close() override;
};


/* 关闭状态 */
class ClosingState : public LiftState
{
public:
    /* 这个应该是是本状态对应的动作 */
    virtual void close() override;

    virtual void open() override;
    virtual void run() override;
    virtual void stop() override;
};

class RunningState : public LiftState
{
public:
    /* 这个应该是是本状态对应的动作 */
    virtual void run() override;

    virtual void stop() override;
};

class StoppingState : public LiftState
{
public:
    /* 这个应该是是本状态对应的动作 */
    virtual void stop() override;

    virtual void open() override;
    virtual void run() override;
};

#endif	/* !__STATE_HPP__ */