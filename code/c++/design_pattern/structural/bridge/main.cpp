/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 桥梁模式
时间	   	: 2023-06-13 21:27
***************************************************************/
#include <iostream>
#include <memory>


class Product
{
public:
    virtual void to_producted() = 0;
    virtual void to_shelled() = 0;
};


class Corp
{
public:
    Corp(std::shared_ptr<Product> product) : 
        product_(product)
    {
    }

public:
    virtual void make_money()
    {
        product_->to_producted();
        product_->to_shelled();
    }

private:
    std::shared_ptr<Product> product_;
};


class HouseCorp : public Corp
{
public:
    using Corp::Corp;

public:
    virtual void make_money()
    {
        this->Corp::make_money();
        std::cout << "房地产公司赚大钱了\n";
    }
};

class ShanZhaiCorp : public Corp
{
public:
    using Corp::Corp;

public:
    virtual void make_money()
    {
        this->Corp::make_money();
        std::cout << "山寨公司赚钱了\n";
    }
};


class House : public Product
{
public:
    void to_producted()
    {
        std::cout << "生产出来的房子是这样的 ...\n";
    }

    void to_shelled()
    {
        std::cout << "生产出来的房子都卖了 ...\n";

    }
};

class Aqqle : public Product
{
public:
    void to_producted()
    {
        std::cout << "生产出来的 Aqqle 是这样的 ...\n";
    }

    void to_shelled()
    {
        std::cout << "生产出来的 Aqqle 都卖了 ...\n";

    }
};

class IPod: public Product
{
public:
    void to_producted()
    {
        std::cout << "生产出来的 IPod 是这样的 ...\n";
    }

    void to_shelled()
    {
        std::cout << "生产出来的 IPod 都卖了 ...\n";

    }
};


int main()
{   
    HouseCorp house_corp(std::make_shared<House>());
    house_corp.make_money();

    ShanZhaiCorp shanzhai_corp_1(std::make_shared<Aqqle>());
    shanzhai_corp_1.make_money();

    ShanZhaiCorp shanzhai_corp_2(std::make_shared<IPod>());
    shanzhai_corp_2.make_money();

    return 0;
}