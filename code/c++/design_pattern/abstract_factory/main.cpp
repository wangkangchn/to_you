/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 抽象工厂模式
时间	   	: 2023-06-11 12:03
***************************************************************/
#include <iostream>
#include <memory>


class Master
{
public:
    virtual ~Master() = default;
};

class Level1Master : public Master
{
public:
    Level1Master()
    {
        std::cout << "我是 1 级怪\n";
    }

    virtual ~Level1Master() = default;
};

class Level2Master : public Master
{
public:
    Level2Master()
    {
        std::cout << "我是 2 级怪\n";
    }
    
    virtual ~Level2Master() = default;
};

class Level3Master : public Master
{
public:
    Level3Master()
    {
        std::cout << "我是 3 级怪\n";
    }

    virtual ~Level3Master() = default;
};


class Cat : public Level1Master
{
public:
    Cat()
    {
        std::cout << "Cat\n";
    }
};

class Dog : public Level1Master
{
public:
    Dog()
    {
        std::cout << "Dog\n";
    }
};

class Tiger : public Level2Master
{
public:
    Tiger()
    {
        std::cout << "Tiger\n";
    }
};

class Lion : public Level2Master
{
public:
    Lion()
    {
        std::cout << "Lion\n";
    }
};

class Angel : public Level3Master
{
public:
    Angel()
    {
        std::cout << "Angel\n";
    }
};

class Daemon : public Level3Master
{
public:
    Daemon()
    {
        std::cout << "Daemon\n";
    }
};


class AbstractSceneFactory
{
public:
    virtual ~AbstractSceneFactory() = default;

public:
    virtual std::shared_ptr<Level1Master> CreateLevel1Master() = 0;
    virtual std::shared_ptr<Level2Master> CreateLevel2Master() = 0;
    virtual std::shared_ptr<Level3Master> CreateLevel3Master() = 0;
};

class Scene1Factory : public AbstractSceneFactory
{
public:
    virtual std::shared_ptr<Level1Master>  CreateLevel1Master()
    {
        return std::make_shared<Cat>();
    }

    virtual std::shared_ptr<Level2Master> CreateLevel2Master()
    {
        return std::make_shared<Tiger>();
    }

    virtual std::shared_ptr<Level3Master> CreateLevel3Master()
    {
        return std::make_shared<Daemon>();
    }
};

class Scene2Factory : public AbstractSceneFactory
{
public:
    virtual std::shared_ptr<Level1Master>  CreateLevel1Master()
    {
        return std::make_shared<Dog>();
    }

    virtual std::shared_ptr<Level2Master> CreateLevel2Master()
    {
        return std::make_shared<Lion>();
    }

    virtual std::shared_ptr<Level3Master> CreateLevel3Master()
    {
        return std::make_shared<Angel>();
    }
};


int main()
{
    std::shared_ptr<AbstractSceneFactory> screen1(new Scene1Factory);
    screen1->CreateLevel3Master();
    screen1->CreateLevel2Master();
    screen1->CreateLevel1Master();

    std::shared_ptr<AbstractSceneFactory> screen2(new Scene2Factory);
    screen2->CreateLevel3Master();
    screen2->CreateLevel2Master();
    screen2->CreateLevel1Master();

    return 0;
}

