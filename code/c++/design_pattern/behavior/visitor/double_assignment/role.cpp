/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: role.hpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 
时间	   	: 2023-07-09 20:57
***************************************************************/
#include <iostream>
#include <memory>

class IActor;
class IRole
{
public:
    virtual ~IRole() = default;

public:
    virtual void accept(std::shared_ptr<IActor> actor) = 0;
};


class KungFuRole;
class IActor
{
public:
    virtual ~IActor() = default;

public:
    virtual void act(std::shared_ptr<IRole> role)
    {
        std::cout << "演员可以扮演任何角色\n";
    }

    virtual void act(std::shared_ptr<KungFuRole> role)
    {
        std::cout << "演员可以扮演功夫角色\n";
    }
};

class KungFuRole : public IRole
{
public:
    virtual void accept(std::shared_ptr<IActor> actor) override
    {
        actor->act(std::shared_ptr<KungFuRole>(this, [] (void*) {}));
    }
};

class YoungActor : public IActor
{
public:
    virtual void act(std::shared_ptr<KungFuRole> role) override
    {
        std::cout << "最喜欢扮演功夫角色\n";
    }
};


int main()
{
    std::shared_ptr<IActor> actor(new YoungActor);
    std::shared_ptr<IRole> role(new KungFuRole);

    // actor->act(role);   /* 这里匹配的是 Role  */
    // actor->act(std::make_shared<KungFuRole>()); /* 这里匹配的是 KungFuRole */

    role->accept(actor);    /* c++ 可以支持双分派诶 */
    return 0;
}