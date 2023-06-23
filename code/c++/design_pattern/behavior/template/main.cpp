/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 模板方法模式一个很经典例子就是线程对象的实现, 
    基类实现线程控制, 子类实现具体的线程要执行的内容
时间	   	: 2023-06-23 11:28
***************************************************************/
#include <iostream>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>


#if RPCLIB_CXX_STANDARD >= 14
    using std::make_unique;
#else
    #include <memory>
    template <class T, class... Args>
    typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type
    make_unique(Args &&...args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
#endif

class ThreadObject
{
public:
    /**
     * @param [in]  detach:     是否创建守护线程
     */
    ThreadObject(bool detach=false) :
        detach_(detach),
        stop_(true),
        pause_(true),
        th_(nullptr)
    {
    }

    virtual ~ThreadObject()
    {   
        stop();
        pause_.store(true);
        if (th_ && th_->joinable()) {
            th_->join();
        }
        pause_.store(true);
        th_ = nullptr;
    }

public:
    void start()
    {
        stop_.store(false);
        pause_.store(false);

        th_ = make_unique<std::thread>(&ThreadObject::run, this);

        if (detach_) {
            th_->detach();  
        }
    }

    void stop()
    {
        stop_.store(true);
    }

    void pause()
    {
        pause_.store(true);
    }

    void resume()
    {
        pause_.store(false);
        cv_.notify_one();
    }


protected:
    /**
     *     当前线程一轮循环所要做到工作
     *
     * @return     true 工作未结束, false 工作结束
     */
    virtual bool work() = 0;

private:
    void run()
    {
        while (!stop_.load()) {
            
            if (pause_) {
                std::unique_lock<std::mutex> lck{mtx_};
                cv_.wait(lck, [this] { return !pause_ || stop_; });
            }

            /* 没有停止才可以继续 */
            if (!stop_ && !work()) {  /* 返回 false 说明工作结束 */
                stop_.store(true);
            }
        }
    }

private:
    bool detach_;
    std::mutex                  mtx_;               /*!< 保护任务队列 */
    std::condition_variable     cv_;
    std::atomic<bool> stop_;
    std::atomic<bool> pause_;
    std::unique_ptr<std::thread> th_;
};


class MyTask : public ThreadObject
{
protected:
    virtual bool work() override
    {   
        static size_t count = 0;
        if (count++ >= 5) {
            std::cout << std::this_thread::get_id() << " 退出\n";
            return false;
        }
        std::cout << std::this_thread::get_id() << " 运行\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return true;
    }
};

int main()
{
    MyTask task;

    std::cout << std::this_thread::get_id() << " task 开始运行\n";
    task.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(4000));

    std::cout << std::this_thread::get_id() << " 要求 task 暂停\n";
    task.pause();
    std::this_thread::sleep_for(std::chrono::milliseconds(4000));

    std::cout << std::this_thread::get_id() << " 要求 task 恢复运行\n";
    task.resume();
    std::this_thread::sleep_for(std::chrono::milliseconds(4000));

    std::cout << std::this_thread::get_id() << " 要求 task 停止\n";
    task.stop();

    return 0;
};