/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 建造者模式
时间	   	: 2023-06-11 17:00
***************************************************************/
#include <iostream>
#include <memory>
#include <string>


class CarPart
{
public:
    virtual ~CarPart() = default;
    virtual void show() = 0;
};

class Engine : public CarPart
{
public:
    virtual ~Engine() = default;
    virtual void show()
    {
        std::cout << "我是最普通的发动机. 牟牟ლ(′◉❥◉｀ლ)\n";
    }
    
};

class V6Engine : public Engine
{
public:
    virtual void show()
    {
        std::cout << "我可是 V6 发动机. 咔咔咔\n";
    }
};

class V12Engine : public Engine
{
public:
    virtual void show()
    {
        std::cout << "我是地表最强 V12 发动机. 唰 ...\n";
    }
};

class Tyre : public CarPart
{
public:
    virtual ~Tyre() = default;
    virtual void show()
    {
        std::cout << "我是最普通的公路轮胎\n";
    }
};

class OffRoadTyre : public Tyre
{
public:
    virtual void show()
    {
        std::cout << "我是越野轮胎, 翻山越岭不在话下\n";
    }
};

class SnowTyre : public Tyre
{
public:
    virtual void show()
    {
        std::cout << "雪地轮胎是也, 走, 溜冰去\n";
    }
};


class Car
{
public:
    Car(std::shared_ptr<Tyre> tyre, std::shared_ptr<Engine> engine) :
        tyre(tyre), engine(engine)
    {}

    /* 模板方法 */
    void run()
    {
        std::cout << "嗡嗡嗡 ...\n 我是装配有 ";
        tyre->show();
        engine->show();
        this->show();
    }

protected:
    virtual void show() const = 0;

private:
    std::shared_ptr<CarPart> tyre;
    std::shared_ptr<CarPart> engine;
};

class Renault : public Car
{
public:
    using Car::Car;

protected:
    virtual void show() const
    {
        std::cout << "雷诺是也\n";
    }
};

class Bumblebee : public Car
{
public:
    using Car::Car;

protected:
    virtual void show() const
    {
        std::cout << "大黄蜂是也\n";
    }
};


class CarBuilder
{
public:
    virtual ~CarBuilder() = default;

public:
    void create_engine(const std::string& name)
    {
        if (name == "v6") {
            engine = std::make_shared<V6Engine>();
        } else if (name == "v12") {
            engine = std::make_shared<V12Engine>();
        } else {
            engine = std::make_shared<Engine>();
        }
    }

    void create_tyre(const std::string& name)
    {
        if (name == "off_road") {
            tyre = std::make_shared<OffRoadTyre>();
        } else if (name == "snow") {
            tyre = std::make_shared<SnowTyre>();
        } else {
            tyre = std::make_shared<Tyre>();
        }
    }

    virtual std::shared_ptr<Car> create_car() = 0;

protected:
    std::shared_ptr<Engine> engine; 
    std::shared_ptr<Tyre> tyre; 
};

class RenaultCarBuilder : public CarBuilder
{
public:
    virtual std::shared_ptr<Car> create_car()
    {
        return std::make_shared<Renault>(tyre, engine);
    }
};

class BumblebeeCarBuilder : public CarBuilder
{
public:
    virtual std::shared_ptr<Car> create_car()
    {
        return std::make_shared<Bumblebee>(tyre, engine);
    }
};


class Director
{
public:
    Director() :
        renault_car_builder(new RenaultCarBuilder), 
        bumblebee_car_builder(new BumblebeeCarBuilder)
    {
    }

    std::shared_ptr<Renault>   create_renault_1()
    {
        renault_car_builder->create_engine("v6");
        renault_car_builder->create_tyre("off_road");
        return std::dynamic_pointer_cast<Renault>(renault_car_builder->create_car());
    }

    std::shared_ptr<Renault>   create_renault_2()
    {
        renault_car_builder->create_engine("v12");
        renault_car_builder->create_tyre("off_road");
        return std::dynamic_pointer_cast<Renault>(renault_car_builder->create_car());
    }

    std::shared_ptr<Renault>   create_renault_3()
    {
        renault_car_builder->create_tyre("snow");
        renault_car_builder->create_engine(" ");
        return std::dynamic_pointer_cast<Renault>(renault_car_builder->create_car());
    }

    std::shared_ptr<Bumblebee>   create_bumblebee_1()
    {
        bumblebee_car_builder->create_engine("v12");
        bumblebee_car_builder->create_tyre("snow");
        return std::dynamic_pointer_cast<Bumblebee>(bumblebee_car_builder->create_car());
    }

    std::shared_ptr<Bumblebee>   create_bumblebee_2()
    {
        bumblebee_car_builder->create_engine("v12");
        bumblebee_car_builder->create_tyre(" ");    /* 不加这个就会用之前的配件, 不是很理想, 没改, 就这样了 */
        return std::dynamic_pointer_cast<Bumblebee>(bumblebee_car_builder->create_car());
    }

    std::shared_ptr<Bumblebee>   create_bumblebee_3()
    {
        bumblebee_car_builder->create_engine(" ");
        bumblebee_car_builder->create_tyre("snow");
        return std::dynamic_pointer_cast<Bumblebee>(bumblebee_car_builder->create_car());
    }

private:
    std::shared_ptr<RenaultCarBuilder> renault_car_builder;
    std::shared_ptr<BumblebeeCarBuilder> bumblebee_car_builder;
};


int main()
{   
    auto director = std::make_shared<Director>();

    director->create_bumblebee_1()->run();
    std::cout << "\n";

    director->create_bumblebee_2()->run();
    std::cout << "\n";

    director->create_bumblebee_3()->run();
    std::cout << "\n";

    director->create_renault_2()->run();
    std::cout << "\n";

    director->create_renault_1()->run();
    std::cout << "\n";

    director->create_renault_3()->run();
    std::cout << "\n";
    return 0;
}