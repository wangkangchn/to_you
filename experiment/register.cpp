/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: register.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 无感注册器
时间	   	: 2023-07-10 21:27
***************************************************************/
#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <unordered_map>


void func(int a)
{
    std::cout << "func1: " << a << std::endl;
}

void func(std::string a)
{
    std::cout << "func1: " << a << std::endl;
}

void func()
{
    std::cout << "func1: 空" << std::endl;
}

void func(float a, std::string b)
{
    std::cout << "func1: " << a << ", " << b << std::endl;
}

class IEvent
{
public:
    virtual ~IEvent() = default;

public:
    virtual void show() const
    {
        std::cout << "空事件\n";
    }
};

class INoCopy
{
protected:
    virtual ~INoCopy() = default;
    INoCopy() = default;
    INoCopy(const INoCopy&) = delete;
    INoCopy& operator=(const INoCopy&) = delete;
    INoCopy(INoCopy&&) = delete;
    INoCopy& operator=(INoCopy&&) = delete;
};

class EventDataBase : public INoCopy
{
public:
    static EventDataBase& getInstance()
    {
        static std::unique_ptr<EventDataBase> instance(new EventDataBase);
        return *instance;
    }

    template <typename Event, typename ...Args>
    void reigster(std::string name, Args&&... args)
    {
        if (events_.count(name) == 0) {
            events_.insert(
                    {name, std::make_shared<Event>(std::forward<Args>(args)...)}
                );
            std::cout << name << "注册成功\n";
        }
    }

    std::shared_ptr<IEvent> operator[](const std::string& name)
    {
        if (events_.count(name)) {
            return events_[name];
        }

        return std::make_shared<IEvent>();
    }


private:
    EventDataBase() = default;

private:
    std::unordered_map<std::string, std::shared_ptr<IEvent>> events_;
};


class Event1 : public IEvent
{
public:
    explicit Event1(int a) : a_(a) {}
    
public:
    virtual void show() const override 
    {
        std::cout << "Event1 a: " << a_ << std::endl;
    }

private:
    int a_;
};

class Event2 : public IEvent
{
public:
    explicit Event2(std::string a) : a_(a) {}

public:
    virtual void show() const override 
    {
        std::cout << "Event2 a: " << a_ << std::endl;
    }

private:
    std::string a_;
};

class Event3 : public IEvent
{
public:
    virtual void show() const override 
    {
        std::cout << "Event3 a: 空" << std::endl;
    }

};

template <typename Event>
class __Register
{
public:
    template <typename ...Args>
    __Register(std::string name, Args&&... args)
    {
        EventDataBase::getInstance().reigster<Event>(std::move(name), std::forward<Args>(args)...);
    }
};

#define REGISTER_EVENT(event_type, ...) \
    static __Register<event_type> __##event_type##__(#event_type, ##__VA_ARGS__)

REGISTER_EVENT(Event1, 1200);
REGISTER_EVENT(Event2, "I'm string");
REGISTER_EVENT(Event3);

#define GET_EVENT(event_name) \
    EventDataBase::getInstance()[#event_name]


int main()
{
    GET_EVENT(Event1)->show();
    GET_EVENT(Event2)->show();
    GET_EVENT(Event3)->show();
    return 0;
}