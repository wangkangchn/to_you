/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 观察者模式
    示例参考自 《设计模式之禅》 第 22 章
时间	   	: 2023-07-03 21:45
***************************************************************/
#include <memory>
#include <string>
#include <iostream>
#include <unordered_set>


/* 观察者 */
class IObserver
{
public:
    virtual ~IObserver() = default;

public:
    /* 观察者被触发时, 所需要做的操作 */
    virtual void update(const std::string& context) = 0;
};

class Observable
{
public:
    virtual ~Observable() = default;

public:
    void add_observer(std::shared_ptr<IObserver> observer)
    {
        observers_.emplace(observer);
    }

    void delete_observer(std::shared_ptr<IObserver> observer)
    {
        observers_.erase(observer);
    }

    /* 通知所有观察者 */
    void notify_observers(const std::string& context)
    {
        for (auto& observer : observers_) {
            observer->update(context);
        }
    }

private:
    std::unordered_set<std::shared_ptr<IObserver>> observers_;
};


class HanFeiZi : public Observable
{
public:
    void have_breakfast()
    {
        std::cout << "韩非子: 我在吃早饭\n";
        notify_observers("韩非子在吃早饭");
    }

    void have_fun()
    {
        std::cout << "韩非子: 我在玩耍\n";
        notify_observers("韩非子在玩耍");
    }
};


class LiSi : public IObserver
{
public:
    virtual void update(const std::string& context) override
    {
        std::cout << "嘿嘿嘿 师弟啊, 别来无恙啊, 你干啥我全知道" << context << std::endl;
    }
};

class WangLaoWu : public IObserver
{
public:
    virtual void update(const std::string& context) override
    {
        std::cout << "你 " << context << " 我就去举报" << std::endl;
    }
};


int main()
{
    auto han_fei_zi = std::make_shared<HanFeiZi>();
    auto li_si = std::make_shared<LiSi>();
    auto wang_lao_wu = std::make_shared<WangLaoWu>();

    han_fei_zi->add_observer(li_si);
    han_fei_zi->add_observer(wang_lao_wu);

    han_fei_zi->have_breakfast();
    han_fei_zi->have_fun();

    return 0;
}