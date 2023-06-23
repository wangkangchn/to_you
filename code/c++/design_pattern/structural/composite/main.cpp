/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 组合模式
        示例参考自 《设计模式之禅》 第 21 章
时间	   	: 2023-06-18 16:58
***************************************************************/
#include <iostream>
#include <vector>
#include <memory>
#include <typeinfo> 

class Corp
{
public:
    Corp(const std::string& name, const std::string& position, int salary) :
        name_(name), position_(position), salary_(salary)
    {
    }

    virtual ~Corp() = default;

public:
    std::string getInfo() const
    {
        std::string info;
        info += "name: " + name_ + ", ";
        info += "position: " + position_ + ", ";
        info += "salary: " + std::to_string(salary_) + "\n";
        return info;
    }

private:
    std::string name_;
    std::string position_;  /* 职位 */
    size_t salary_;
    std::vector<std::shared_ptr<Corp>> subordinate_;    /* 下属 */
};

class Leaf : public Corp
{
public:
    using Corp::Corp;
};

/* 这种实现是安全模式, 将操作下放到子类中, 而不全部在基类中定义!!!
全部在基类中进行定义称之为透明模式 
*/
class Branch : public Corp
{
public:
    using Corp::Corp;

public:
    void addSubordinate(std::shared_ptr<Corp> corp)
    {
        subordinate_.emplace_back(corp);
    }

    std::vector<std::shared_ptr<Corp>> getSubordinate()
    {
        return subordinate_;
    }

private:
    std::vector<std::shared_ptr<Corp>> subordinate_;    /* 下属 */
};


/* 遍历树 */
std::string getTreeInfo(std::shared_ptr<Branch> root)
{
    auto subordinate = root->getSubordinate();
    std::string info;

    for (auto& s: subordinate) {
        info += s->getInfo() + "\n";
        if (typeid(Branch) == typeid(*s.get())) {
            info += getTreeInfo(std::dynamic_pointer_cast<Branch>(s));
        }
    }
    return info;
}

int main()
{


    auto root = std::make_shared<Branch>("王大麻子", "总经理", 100000);
    auto develop_dep = std::make_shared<Branch>("马二拐子", "销售部门经理", 10000);
    auto sales_dep = std::make_shared<Branch>("赵三驼子", "财务部经理", 10000);

    auto a = std::make_shared<Leaf>("a", "开发人员", 1000);
    auto b = std::make_shared<Leaf>("b", "开发人员", 1000);
    auto c = std::make_shared<Leaf>("c", "开发人员", 1000);

    root->addSubordinate(develop_dep);
    root->addSubordinate(sales_dep);

    develop_dep->addSubordinate(a);
    develop_dep->addSubordinate(b);
    sales_dep->addSubordinate(c);

    std::cout << root->getInfo() << std::endl;
    std::cout << getTreeInfo(root) << std::endl;
    return 0;
}