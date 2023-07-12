/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 解释器模式
    示例参考自 《设计模式之禅》 第 27 章
时间	   	: 2023-07-12 21:34
***************************************************************/
#include <iostream>
#include <memory>
#include <stack>
#include <unordered_map>


/* 抽象表达式, 进行加减法运算 */
class Expression
{
public:
    virtual ~Expression() = default;

public:
    virtual int interpreter(const std::unordered_map<char, int>& var) = 0;
};


/**
 *  变量表达式
 */
class VarExpression : public Expression
{
public:
    explicit VarExpression(char key) :
        key_(key)
    {
    }

public:
    virtual int interpreter(const std::unordered_map<char, int>& var) override
    {
        return var.at(key_);
    }

private:
    char key_;
};

/* 运算符表达式 */
class SymbolExpression : public Expression
{
public:
    SymbolExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right) :
        left_(left), right_(right)
    {
    }

protected:
    std::shared_ptr<Expression> left_;  /* 应该存储的是变量 */
    std::shared_ptr<Expression> right_;
};

class AddExpression : public SymbolExpression
{
public:
    using SymbolExpression::SymbolExpression;

public:
    virtual int interpreter(const std::unordered_map<char, int>& var) override
    {
        return left_->interpreter(var) + right_->interpreter(var);
    }
};

class SubExpression : public SymbolExpression
{
public:
    using SymbolExpression::SymbolExpression;

public:
    virtual int interpreter(const std::unordered_map<char, int>& var) override
    {
        return left_->interpreter(var) - right_->interpreter(var);
    }
};


class Calculator
{
public:
    Calculator(const std::string& exp)
    {
        std::stack<std::shared_ptr<Expression>> stack;
        std::shared_ptr<Expression> left = nullptr, right = nullptr;
        
        /* ooo 它是先一直存一直存, 先把表达式存下来, 一个表达式结果放到另一个表达式中,
        而后在传入变量, 递归回退这样就能求解出来, 妙啊 */
        for (int i = 0; i < exp.size(); ++i) {
            switch (exp[i]) {
                case '+':
                    left = stack.top();
                    stack.pop();
                    right = std::make_shared<VarExpression>(exp[++i]);
                    stack.push(std::make_shared<AddExpression>(left, right));
                    break;

                case '-':
                    left = stack.top();
                    stack.pop();
                    right = std::make_shared<VarExpression>(exp[++i]);
                    stack.push(std::make_shared<SubExpression>(left, right));
                    break;
                
                default:
                    stack.push(std::make_shared<VarExpression>(exp[i]));
                    break;
            }
        }

        expression_ = stack.top();
        stack.pop();
    }

public:
    int run(const std::unordered_map<char, int>& var)
    {
        return expression_->interpreter(var);
    }

private:
    std::shared_ptr<Expression> expression_;
};


int main()
{
    std::string exp{"a+b-c"};
    std::unordered_map<char, int> var {
        {'a', 100},
        {'b', 20},
        {'c', 30}
    };

    std::shared_ptr<Calculator> cal(new Calculator(exp));
    std::cout << exp << " = " << cal->run(var) << std::endl;
    return 0;
}