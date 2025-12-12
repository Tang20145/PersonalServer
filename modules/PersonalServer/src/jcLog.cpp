#include "jcLog.h"

// 全局日志对象
std::map<std::string,std::shared_ptr<spdlog::logger>> jcLog::g_vLoggerManager;
std::shared_ptr<spdlog::logger> jcLog::g_mainLogger;
std::shared_ptr<spdlog::logger> jcLog::g_sqlLogger;

// 定时flush日志线程----------------------------------

std::atomic<bool> jcLog::g_bStopFlushThread;
std::unique_ptr<std::thread> jcLog::g_thFlushThread;

void jcLog::vFlusherThreadFunc(const std::map<std::string,std::shared_ptr<spdlog::logger>>& p_mapLoggers,std::chrono::seconds l_interval)
{
while (!g_bStopFlushThread.load()) {
            // 线程睡眠指定间隔，并检查停止标志
            // 使用 sleep_until 或 wait_for 是更优雅且能响应中断的方式
            std::this_thread::sleep_for(l_interval);

            if (g_bStopFlushThread.load()) 
                break; // 线程被唤醒后，如果标志已设置，则退出循环
            
            // flush
            for(auto it : p_mapLoggers)
            {
                it.second->flush();
            }

            // 可以在这里添加一些调试输出
            // std::cout << "Log flush executed." << std::endl;
        }
        std::cout << "Log flusher thread stopped gracefully." << std::endl;
}

void jcLog::vStartFlushing(const std::map<std::string,std::shared_ptr<spdlog::logger>>& p_mapLoggers, std::chrono::seconds l_interval)
{
    if (g_thFlushThread.get() != nullptr) {
        // 已经运行了
        return; 
    }

    g_bStopFlushThread.store(false);
    
    g_thFlushThread = std::make_unique<std::thread>(jcLog::vFlusherThreadFunc, p_mapLoggers,l_interval);
}

void jcLog::vStopFlushing()
{
    if (g_bStopFlushThread.load() && g_thFlushThread->joinable()) {
        g_bStopFlushThread.store(true); // 设置停止标志
        
        // 等待线程执行完最后一次循环并退出
        g_thFlushThread->join();
        g_thFlushThread.reset(); // 释放线程资源
    }
}


// END 定时flush日志线程------------------------------

// 初始化日志函数
void jcLog::vJcLogInitAsyncLogger(std::shared_ptr<spdlog::logger> & p_logger , const std::string & l_strLoggerName ,const std::string& l_strLogFileName)
{
    try
    {        
        // 基础日志对象
        // p_logger = spdlog::basic_logger_mt<spdlog::async_factory>(l_strLoggerName, l_strLogFileName);
        // 随日期变更的日志对象
        p_logger = spdlog::daily_logger_mt(l_strLoggerName,l_strLogFileName,0,0);
        g_vLoggerManager[l_strLoggerName] = p_logger;
        // Under VisualStudio, this must be called before main finishes to workaround a known VS issue
        // spdlog::drop_all(); 
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }
}