/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 职责链模式
    示例参考自 《设计模式之禅》 第 16 章
时间	   	: 2023-07-10 20:46
***************************************************************/
#include <memory>
#include <iostream>


class Request
{
public:
    virtual ~Request() = default;
    Request(int level) : level_(level) {}

public:
    int getRequestLevel() const
    {
        return level_;
    }

private:
    int level_;
};


class Repose
{
public:
    explicit Repose(const std::string& msg="没有响应") :
        msg_(msg)
    {
    }

public:
    std::string msg() const { return msg_; }

private:
    std::string msg_;
};


class Handler
{
public:
    virtual ~Handler() = default;

public:
    /* 下一个处理者 */
    void setNext(std::shared_ptr<Handler> handler)
    {
        next_handler_ = handler;
    }   


    Repose handle(std::shared_ptr<Request> request)
    {
        if (request->getRequestLevel() == getHandlerLeven()) {
            return repose();
        } else {
            if (next_handler_) {
                return next_handler_->handle(request);
            } else {
                return Repose();
            }
        }
    }

    virtual int getHandlerLeven() const = 0;
    virtual Repose repose() = 0;


private:
    std::shared_ptr<Handler> next_handler_;
};


class ConcreteHandler1 : public Handler
{
public:
    virtual int getHandlerLeven() const
    {
        return 1;
    }

    virtual Repose repose() override
    {
        return Repose("ConcreteHandler1 进行响应");
    }
};

class ConcreteHandler2: public Handler
{
public:
    virtual int getHandlerLeven() const
    {
        return 2;
    }

    virtual Repose repose() override
    {
        return Repose("ConcreteHandler2 进行响应");
    }
};

class ConcreteHandler3: public Handler
{
public:
    virtual int getHandlerLeven() const
    {
        return 3;
    }

    virtual Repose repose() override
    {
        return Repose("ConcreteHandler3 进行响应");
    }
};



int main()
{
    std::shared_ptr<Handler> handler1(new ConcreteHandler1);
    std::shared_ptr<Handler> handler2(new ConcreteHandler2);
    std::shared_ptr<Handler> handler3(new ConcreteHandler3);
    handler1->setNext(handler2);
    handler2->setNext(handler3);

    std::cout << handler1->handle(std::make_shared<Request>(3)).msg() << std::endl;
    std::cout << handler1->handle(std::make_shared<Request>(2)).msg() << std::endl;
    std::cout << handler1->handle(std::make_shared<Request>(1)).msg() << std::endl;

    return 0;
}