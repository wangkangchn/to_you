/***************************************************************
 * @copyright  Copyright © 2023 wkangk.
 * @file       4_request_handler.cpp
 * @author     wkangk <wangkangchn@163.com>
 * @version    v1.0
 * @brief      请求处理
 * @date       2023-07-27 21:09
 **************************************************************/
#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_map>

/**
 *     HTTP 响应返回的数据
 */
struct ResultData
{
    ResultData(const std::string& content, const std::string& content_type) :
        m_Content(content), m_ContentType(content_type)
    {}

    std::string m_Content;          /*!< 结果的具体内容 */
    std::string m_ContentType;      /*!< 结果类型 如: application/json image/png
                                        HTTP 响应所需的类型 */
};

/**
 *     HTTP 请求处理器
 * 针对不同请求, 需继承该类进行处理, 并将结果转换为指定格式
 */
class IRequestHandler
{
public:
    virtual ~IRequestHandler() = default;

public:
    /**
     *     执行具体的请求处理
     */
    virtual ResultData execute() = 0;
};

/**
 * 对请求数据添加额外信息
 */
class ExtraInformationDecorator : public IRequestHandler
{
public:
    ExtraInformationDecorator(std::shared_ptr<IRequestHandler> handler) : m_Handler(handler) {}

public:
    virtual ResultData execute() override 
    {   
        return addExtraInfo(m_Handler->execute()); 
    }

private:
    /* 添加额外信息 */
    ResultData addExtraInfo(ResultData data)
    {
        /* 
            添加额外信息
        
         */
        return data;
    }

private:
    std::shared_ptr<IRequestHandler> m_Handler;

    std::unordered_map<std::string, std::string> m_ExtraInfo = {
        {"project", "hpc"},
        {"vector", "v1.0.0"}
    };
};

/**
 * 调用处理器
 */
class Invoker 
{
public:
    static ResultData action(std::shared_ptr<IRequestHandler> handler) 
    {
        return handler->execute();
    }
};


typedef std::string json;

typedef json TextData;

/* 允许产生多个媒体数据, 键为媒体数据名称, 值为具体数据 */
typedef std::unordered_map<std::string, ResultData> MediaData;

/**
 * 媒体重定向, 将原始媒体数据替换为媒体 url
 */
class IMediaRedirector
{
public:
    virtual ~IMediaRedirector() = default;

public:
    virtual TextData& redirect(TextData& text, MediaData media) = 0;
};

class MediaRedirector : public IMediaRedirector
{
public:
    virtual TextData& redirect(TextData& text, MediaData media) override { return text; }
};

class IExtraInfosWrapper
{
public:
    virtual ~IExtraInfosWrapper() = default;

public:
    virtual TextData warp(TextData data, std::exception_ptr ep) = 0;
    virtual ResultData warp(ResultData data, std::exception_ptr ep) = 0;
};

class ExtraInfosWrapper : public IExtraInfosWrapper
{
public:
    virtual TextData warp(TextData data, std::exception_ptr ep) override { return data; }
    virtual ResultData warp(ResultData data, std::exception_ptr ep) override { return data;}
};

/**
 *  请求媒体数据的时候, 相应处理器要继承此类
 *  在获取媒体数据后, 会将数据进行重定向, 使用媒体 url 代替原先媒体数据
 */
class IJsonWithMediaRequestHandler : public IRequestHandler
{
public:
    IJsonWithMediaRequestHandler() :
        m_MediaRedirector(new MediaRedirector),
        m_ExtraInfosWrapper(new ExtraInfosWrapper)
    {}

    virtual ~IJsonWithMediaRequestHandler() = default;

public: 
    /**
     *  不再允许子类继续重写该方法, 因为会在此处进行媒体数据的重定向
     */
    virtual ResultData execute() override final
    {
        TextData text_data;
        std::exception_ptr ep;

        try {
            work();
            text_data = getTextData();
            text_data = m_MediaRedirector->redirect(text_data, getMediaData()); /* 资源重定向 */
        } catch (...) {
            ep = std::current_exception();
        }

        return warp(std::move(text_data), std::move(ep));                       /* 信息包装器 */
    }

    void setMediaRedirector(std::shared_ptr<IMediaRedirector> redirector) { m_MediaRedirector = redirector; }
    void setExtraInfosWrapper(std::shared_ptr<IExtraInfosWrapper> wrapper) { m_ExtraInfosWrapper = wrapper; }

protected:
    /**
     *  需要实现的新处理方法, 代替父类的 execute, 该方法会在父类中
     * 调用
     */
    virtual void work() = 0;

    /**
     *     work 完成后需要使用下面两个方法来获取文本数据与媒体数据
     *  在 execute 中 会通过此两个方法获取所需数据进行资源重定向
     */
    virtual TextData getTextData() = 0;

    virtual MediaData getMediaData() { return {}; }

private:
    ResultData warp(TextData text_data, std::exception_ptr ep)
    {
        return ResultData(m_ExtraInfosWrapper->warp(std::move(text_data), ep), "application/json");
    }

private:
    std::shared_ptr<IMediaRedirector> m_MediaRedirector;
    std::shared_ptr<IExtraInfosWrapper> m_ExtraInfosWrapper;
};

class A
{
public:
    A() {std::cout << "A" << std::endl;}
    A(const A&) {std::cout << "拷贝构造A" << std::endl;}
    // A(A&&) = default;
    A(A&&) {std::cout << "移动构造A" << std::endl;}
};

A foo(A a)
{
    return std::move(a);
}

A foo()
{   
    A a;
    return a;
}
int main()
{
    A a;
    A aa = foo(std::move(a));  /* 只有一次构造 */

    A aaa = foo();
    return 0;
}