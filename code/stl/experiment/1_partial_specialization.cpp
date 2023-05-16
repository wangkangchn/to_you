/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: partial.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 测试模板偏特化
时间	   	: 2023-05-16 20:51
***************************************************************/
#include <iostream>

using namespace std;

template <typename T, typename C>
struct A
{
    A() { cout<< "泛化版本构造函数" << endl;}
    void func()
    {
        cout << "泛化版本" << endl;    
    }
};

template struct A<int, int>;
// template<>
// struct A<int, int>
// {
//     A() { cout << "int,int特化版本构造函数" << endl; }
//     void func()
//     {
//         cout << "int,int特化版本" <<endl;    
//     } 
// };

template<>
struct A<double,double>
{
   A(){cout<<"double,double特化版本构造函数"<<endl;}
   void func()
   {
       cout<<"double,double特化版本"<<endl;    
   } 
};

template<>
void A<int,double>::func()
{
    cout<<"int,double特化版本函数"<<endl;
}


template <typename C>
struct A<char, C>
{
	void func()
	{
		cout << "char, C 偏特化版本" << endl;
	}
};

template <typename T,typename C>
struct A<T*, C*>
{
	void func()
	{
		cout << "T*, C* 偏特化版本" << endl;
	}
};

template<typename T,typename C>
void func(T &a, C &b)
{
	cout << "------------------------" << endl;
	cout << "泛化版本" << endl;
	cout << a << endl;
	cout << b << endl;
	cout << "------------------------" << endl;
}

template<typename C>
void func(char a, C b)
{
	cout << "------------------------" << endl;
	cout << "泛化版本" << endl;
	cout << a << endl;
	cout << b << endl;
	cout << "------------------------" << endl;
}

int main()
{
    A<int,double> a;//调用泛化构造函数
    a.func();//调用泛化版本函数
    A<int,int> a1;//调用int,int特化构造函数
    a1.func();//调用int,int特化版本函数
    A<double,double> a2;//调用double,double特化构造函数
    a2.func();//调用double,double特化版本函数

    A<int,double> a3;//调用double,double特化构造函数
    a3.func();//调用double,double特化版本函数

    /* 偏特化 */
    A<char, char> a4;
    a4.func();

    A<char*, char*> a5;//调用double,double特化构造函数
    a5.func();//调用double,double特化版本函数
    
    // func<char, int>("a", 1);

    struct Base
    {
        ~Base() { std::cout << "Base " << std::endl; }
    };
    
    struct Child : public Base
    {
        ~Child() { std::cout << "Child " << std::endl; }
    };

    std::cout << "sizeof(Base): " << sizeof(Base) << std::endl;
    std::cout << "sizeof(Child): " << sizeof(Child) << std::endl;

    Child ff;

    Base* c = new Child;
    delete c;
    std::cout << "----------------------------------\n";
    return 0;
}