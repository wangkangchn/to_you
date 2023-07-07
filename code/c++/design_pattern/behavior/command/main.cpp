/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 命令模式
    示例参考自 《设计模式之禅》 第 15 章
时间	   	: 2023-07-07 21:16
***************************************************************/
#include <iostream>
#include <memory>

class Command
{
public:
    virtual ~Command() = default;

public:
    virtual void execute() = 0;
};


class Group
{
public:
    virtual ~Group() = default;

public:
    virtual void find() = 0;        /* 找到相应组 */
    virtual void add() = 0;         /* 添加功能 */
    virtual void del() = 0;         /* 删除功能*/
    virtual void change() = 0;         /* 变更功能*/
    virtual void plane() = 0;         /* 给出所有变更计划 */
};

/* 需求组 */
class RequirementGroup : public Group
{
public:
    virtual void find() override
    {
        std::cout << "找到需求组\n";
    }

    virtual void add() override
    {
        std::cout << "客户要求增加一项需求\n";
    }

    virtual void del() override
    {
        std::cout << "客户要求删除一项需求\n";
    }

    virtual void change() override
    {
        std::cout << "客户要求更改一项需求\n";
    }

    virtual void plane() override
    {
        std::cout << "客户要求需求变更计划\n";
    }
};

/* 代码组 */
class CodeGroup : public Group
{
public:
    virtual void find() override
    {
        std::cout << "找到代码组\n";
    }

    virtual void add() override
    {
        std::cout << "客户要求增加一项功能\n";
    }

    virtual void del() override
    {
        std::cout << "客户要求删除一项功能\n";
    }

    virtual void change() override
    {
        std::cout << "客户要求更改一项功能\n";
    }

    virtual void plane() override
    {
        std::cout << "客户要求代码变更计划\n";
    }
};


class Invoker
{
public:
    void setCommand(std::shared_ptr<Command> command)
    {
        command_ = command;
    }

    void action()
    {
        command_->execute();
    }

private:
    std::shared_ptr<Command> command_;
};


class AddRequirementCommand : public Command
{
public:
    AddRequirementCommand() :
        group_(new RequirementGroup)
    {
    }

public:
    virtual void execute() override
    {
        group_->find();
        group_->del();
        group_->plane();
    }

private:
    std::shared_ptr<Group> group_;
};

class AddFunctionCommand : public Command
{
public:
    AddFunctionCommand() :
        group_(new CodeGroup)
    {
    }

public:
    virtual void execute() override
    {
        group_->find();
        group_->del();
        group_->plane();
    }

private:
    std::shared_ptr<Group> group_;
};


int main()
{   
    auto invoker = std::make_shared<Invoker>();
    invoker->setCommand(std::make_shared<AddFunctionCommand>());
    invoker->action();

    invoker->setCommand(std::make_shared<AddRequirementCommand>());
    invoker->action();
    return 0;
}