/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 代理模式
    示例参考自 《设计模式之禅》 第 12 章
动态代理没搞清楚, 没搞清楚啊, 哈哈哈, 罢了, 一点点来吧, 先不搞它了
时间	   	: 2023-06-23 10:26
***************************************************************/
#include <iostream>
#include <memory>
#include <ctime>
#include <random>


class IGamePlayer
{
public:
    virtual ~IGamePlayer() = default;

public:
    virtual void login(const std::string& user, const std::string& password) = 0;
    virtual void kill_boss() = 0;
    virtual void upgrade() = 0;
};

class GamePlayer : public IGamePlayer
{
public:
    GamePlayer(const std::string& name) :
        name_(name)
    {
    }

public:
    virtual void login(const std::string& user, const std::string& password) override
    {
        std::cout << "登录名为: " << user << std::endl;
        std::cout << "密码为: " << password << std::endl;
    }

    virtual void kill_boss() override
    {
        std::cout << name_ << " 在砍老怪 ...\n";
    }

    virtual void upgrade() override
    {
        std::cout << name_ << " 又升一级\n";
    }

private:
    std::string name_;
};


/**
 *     代理类和被代理类属于同一基类下的子类
 */
class GamePlayerProxy : public IGamePlayer
{
public:
    GamePlayerProxy(IGamePlayer* player) :
        game_player_(player)
    {
    }

    virtual void login(const std::string& user, const std::string& password) override
    {
        /* 因为拥有同样的接口, 所以可以直接调用, 
        但是一般代理更注重的是代理行为的控制, 这里有一些行为更好 */
        game_player_->login(user, password);
    }


    virtual void kill_boss() override
    {   
        /* 一般是会有行为的控制 */
        if (rand() % 3 < 1) {
            std::cout << "找到 boss" << std::endl;
            game_player_->kill_boss();
        } else {
            std::cout << "没找到 boss\n";
        }

    }

    virtual void upgrade() override
    {
        game_player_->upgrade();
    }

private:
    IGamePlayer* game_player_;
};


int main()
{
    srand(time(NULL));

    std::unique_ptr<IGamePlayer> player(new GamePlayer("wkangk"));
    std::unique_ptr<IGamePlayer> proxy(new GamePlayerProxy(player.get()));

    proxy->login("wkangk", "qiqi");
    proxy->kill_boss();
    proxy->kill_boss();
    proxy->kill_boss();
    proxy->kill_boss();
    proxy->upgrade();

    return 0;
}