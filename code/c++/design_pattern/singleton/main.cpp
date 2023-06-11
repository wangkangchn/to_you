/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 单例
时间	   	: 2023-06-11 15:17
***************************************************************/
#include <iostream>
#include <memory>


class INoCopy
{
public:
    virtual ~INoCopy() = default;
    INoCopy() = default;
    INoCopy(const INoCopy&) = delete;
    INoCopy& operator=(const INoCopy&) = delete;
    INoCopy(INoCopy&&) = delete;
    INoCopy& operator=(INoCopy&&) = delete;
};

class Singleton : public INoCopy
{
public:
    static Singleton* get_instance()
    {
        /* 因为是类内所以可以直接访问默认构造, 但是类外就不可以了, 所以可以不对
        new 进行处理 */
        /* static 执行到的时候才会初始化, 所以这里并不需要设置为 nullptr 后
        再进行赋值判断 */
        static std::unique_ptr<Singleton> instance(new Singleton);  
        return instance.get();
    }

    void show() const
    {
        std::cout << "hahahhahahah\n";
    }

private:
    Singleton() = default;
};


int main()
{   
//     Singleton* instance = new Singleton();
//     Singleton instance2;
    Singleton::get_instance()->show();
    return 0;
}