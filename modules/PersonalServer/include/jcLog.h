// 避免编译过滤TRACE
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include <iostream>
#include "spdlog/spdlog.h"
#include "spdlog/async.h" //support for async logging.
//#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"

// 封装日志
#ifndef JCLOG_H_
#define JCLOG_H_


namespace jcLog
{
    // 全局日志对象
    extern std::map<std::string,std::shared_ptr<spdlog::logger>> g_vLoggerManager;// 全局日志队列
    extern std::shared_ptr<spdlog::logger> g_mainLogger;
    extern std::shared_ptr<spdlog::logger> g_sqlLogger;

    // 定时flush日志线程----------------------------------
    // 控制刷新线程是否运行的原子标志
    extern std::atomic<bool> g_bStopFlushThread;
    // 刷新线程的句柄
    extern std::unique_ptr<std::thread> g_thFlushThread;
    // 默认的刷新间隔 (每 3 秒刷新一次)
    constexpr std::chrono::seconds DEFAULT_FLUSH_INTERVAL = std::chrono::seconds(3);
    
    /**
     * @brief 刷新线程的主函数。
     * * @param interval 刷新的时间间隔。
     * @param loggers 需要刷新的日志对象列表。
     */
    void vFlusherThreadFunc(const std::map<std::string,std::shared_ptr<spdlog::logger>>& p_mapLoggers,std::chrono::seconds l_interval = DEFAULT_FLUSH_INTERVAL);

    /**
     * @brief 启动日志刷新线程。
     * * @param loggers 所有需要周期性刷新的全局 logger 对象。
     * @param interval 刷新间隔，默认为 3 秒。
     */
    void vStartFlushing(const std::map<std::string,std::shared_ptr<spdlog::logger>>& l_mapLoggers, std::chrono::seconds l_interval = DEFAULT_FLUSH_INTERVAL);

    /**
     * @brief 停止日志刷新线程并等待其结束。
     */
    void vStopFlushing();

    // END 定时flush日志线程------------------------------

    // 初始化日志函数
    void vJcLogInitAsyncLogger(std::shared_ptr<spdlog::logger> & p_logger , const std::string & l_strLoggerName ,const std::string& l_strLogFileName);

}

// 全局日志对象
#define MAIN_LOG    jcLog::g_mainLogger
#define SQL_LOG     jcLog::g_sqlLogger

#endif