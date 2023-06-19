/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 享元模式
时间	   	: 2023-06-19 21:15
***************************************************************/
#include <string>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <chrono>

using namespace std::chrono;


class Flyweight
{
public:
    Flyweight(std::string intrinsic) :
        intrinsic(std::move(intrinsic))
    {
    }

    virtual ~Flyweight() = default;

public:
    /* 业务操作 */
    virtual void opreate() = 0;

    void set_extrinsic(std::string extrinsic)
    {
        this->extrinsic = std::move(extrinsic);
    }

    std::string get_extrinsic() const
    {
        return extrinsic;
    }


private:
    std::string intrinsic;      /* 内部状态, 暂且认定这个是不变量 */
    std::string extrinsic;
};


class ConcreteFlyweight1 : public Flyweight
{
public:
    ConcreteFlyweight1(std::string initstate) :
        Flyweight(std::move(initstate))
    {
        std::string msg;
        for (std::size_t i = 0; i < 100000; ++i) {
            msg += std::to_string(i);
        }

        set_extrinsic(msg);
    }

public:
    virtual void opreate() override
    {
        std::cout << "ConcreteFlyweight1 操作" << std::endl;
    }
};

class ConcreteFlyweight2 : public Flyweight
{
public:
    using Flyweight::Flyweight;

public:
    virtual void opreate() override
    {
        std::cout << "ConcreteFlyweight2 操作" << std::endl;
    }
};


/* 类似传统的工厂模式! 区别在于普通工厂的键在于一族类的标识
而享元工厂的键是对象的内部属性 */
class FlyweightFractory
{
public:
    static std::shared_ptr<Flyweight> get_flyweight(std::string intrinsic)
    {   
        static std::unordered_map<std::string, std::shared_ptr<Flyweight>> pool;

        /* 存在就用之前的, 不存在就新创建 */
        if (pool.count(intrinsic) == 0) {
            pool.insert({intrinsic, std::make_shared<ConcreteFlyweight1>(intrinsic)});
        }

        return pool.at(intrinsic);
    }
};


int main()
{
    std::string key1 = "科目1青岛";
    std::string key2 = "科目2北京";

    auto t1 = steady_clock::now();

    for (size_t i = 0; i < 1000; ++i) {
        auto obj1 = FlyweightFractory::get_flyweight(key1);
        auto obj2 = FlyweightFractory::get_flyweight(key2);
    }

    auto t2 = steady_clock::now();
    auto time_used = duration_cast<duration<double>> (t2 - t1);

    std::cout << "享元工厂用时: " << time_used.count() * 1000 << " ms" << std::endl;

    t1 = steady_clock::now();

    for (size_t i = 0; i < 1000; ++i) {
        auto obj1 = std::make_shared<ConcreteFlyweight1>(key1);
        auto obj2 = std::make_shared<ConcreteFlyweight1>(key2);
    }

    t2 = steady_clock::now();
    time_used = duration_cast<duration<double>> (t2 - t1);
    std::cout << "普通创建对象用时: " << time_used.count() * 1000 << " ms" << std::endl;

    /* 如果构造析构很耗时的, 效率感人, 贼强! */
    /* 
    享元工厂用时: 13.2583 ms
    普通创建对象用时: 12598.6 ms
     */
    return 0;
}