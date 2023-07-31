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

    /**
     *     获取文本数据, 
     */
    virtual TextData getTextData() { return ResultData(); }
    
    /**
     *     获取媒体数据
     *
     * @return     因为一个处理可能会存在多个需要返回的媒体数据
     *          所以这里返回一个媒体数据的 map
     *          其中键为该媒体数据的名字, 值为具体媒体数据
     */
    virtual MediaData getMediaData() { return {}; }
};


/**
 * 请求处理装饰器
 */
class IRequestHandlerDecorator : public IRequestHandler
{
public:
    IRequestHandlerDecorator(std::shared_ptr<IRequestHandler> handler) : m_Handler(handler) {}
    virtual ~IRequestHandlerDecorator() = default;

private:
    virtual void execute() override { m_Handler->execute(); }
    virtual TextData getTextData() override { return m_Handler->getTextData(); }
    virtual MediaData getMediaData() override { return m_Handler->getMediaData(); }

protected:
    std::shared_ptr<IRequestHandler> m_Handler;
};


const std::unordered_map<std::string, std::string> extra_info = {
    {"project", "hpc"},
    {"vector", "v1.0.0"}
};


/**
 * 对请求数据添加额外信息
 */
class ExtraInformationDecorator : public IRequestHandlerDecorator
{
public:
    using IRequestHandlerDecorator::IRequestHandlerDecorator;

public:
    virtual TextData getTextData() override 
    {   
        return addExtraInfo(m_Handler->getTextData()); 
    }

private:
    /* 添加额外信息 */
    TextData addExtraInfo(TextData text)
    {
        /* 
        
        
         */
        return text;
    }
};


/**
 * 媒体重定向, 将原始媒体数据替换为媒体 url
 */
class MediaRedirectDecorator : public IRequestHandlerDecorator
{
public:
    using IRequestHandlerDecorator::IRequestHandlerDecorator;

public:
    virtual void execute() override
    {
        m_Handler->execute();
        m_TextData = m_Handler->getTextData();
        m_MediaData = m_Handler->getMediaData();

        /* 进行重定向处理 */
    }

    virtual TextData getTextData() override { return std::move(m_TextData); }
    virtual MediaData getMediaData() override { return std::move(m_MediaData); }

private:
    TextData m_TextData;
    MediaData m_MediaData;
};


/**
 *      请求处理的执行者
 *  因为 HTTP 一次只能返回一个 Content, 所以需要通过此类将请求处理
 * 的两种类型处理为一种使其可以直接作为结果返回
 */
class IRequestHandlerInvoker
{
public:
    /**
     * @param [in]  handler     需要执行的处理器
     */
    IRequestHandlerInvoker(std::shared_ptr<IRequestHandler> handler) : m_Handler(handler) {}
    virtual ~IRequestHandlerInvoker() = default;

public:
    virtual ResultData action() = 0;

protected:
    std::shared_ptr<IRequestHandler> m_Handler;
};


/**
 * 调用处理器, 并返回处理的文本数据
 */
class TextInvoker : public IRequestHandlerInvoker
{
public:
    using IRequestHandlerInvoker::IRequestHandlerInvoker;

public:
    virtual ResultData action() override
    {
        m_Handler->execute();
        return m_Handler->getTextData();
    }
};

/**
 * 调用处理器, 并返回处理的媒体数据
 */
class MediaInvoker : public IRequestHandlerInvoker
{
public:
    using IRequestHandlerInvoker::IRequestHandlerInvoker;

public:
    /**
     * 当存在多个媒体数据时, 一次仅返回一个
     */
    virtual ResultData action() override
    {
        m_Handler->execute();
        return m_Handler->getMediaData().begin()->second;
    }
};



int main()
{
    return 0;
}