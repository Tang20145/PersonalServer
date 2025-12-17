#include "sqlApi.h"
#include "string.h"
#include <fstream>
#include <iostream>
#include <vector>
#include "commonUse.h"
// 日志库封装
#include "jcLog.h"
// 错误码
#include "jcErrCode.h"

using namespace std;

// 协议传入参数map
// "sql":sql语句
// "page":
// "pageSize":
// "orderDirection":asc / desc
// "orderBy":

// 防止与mysql的string产生歧义
using string = std::string;

namespace sqlApi
{

    // mysqlx::Session *g_nSess = nullptr; // 移除全局会话

    // 初始化
    int init()
    {
        // // 读取json配置文件
        // std::ifstream l_fConfig("config.json");
        // json l_jConfig = json::parse(l_fConfig);

        // {
        //     string host = l_jConfig["database"]["host"];
        //     string username = l_jConfig["database"]["username"];
        //     string password = l_jConfig["database"]["password"];
        //     string databaseName = l_jConfig["database"]["databaseName"];
        //     // 初始化会话
        //     if (g_nSess != nullptr)
        //     {
        //         g_nSess->close();
        //         delete g_nSess;
        //         g_nSess = nullptr;
        //     }

        //     try
        //     {
        //         SPDLOG_LOGGER_INFO(SQL_LOG,"start connect mysql Server using host:{} port:{} username:{} password:{} databaseName:{}",host,33060,username,password,databaseName);
        //         g_nSess = new mysqlx::Session(host, 33060, username, password, databaseName);
                
        //         // std::cout << "init session success!\n";
        //     }
        //     catch (const mysqlx::Error &e)
        //     {
        //         std::cerr << e.what() << "\n"; // mysqlx的异常
        //         return -1;
        //     }
        //     SPDLOG_LOGGER_INFO(SQL_LOG,"init session success!");
        // }
        // return 0;

        std::ifstream l_fConfig("config.json");
        json l_jConfig = json::parse(l_fConfig);
        
        // 构造配置结构
        sqlApi::DbConfig config;
        config.host = l_jConfig["database"]["host"];
        config.port = 33060;
        config.user = l_jConfig["database"]["username"];
        config.password = l_jConfig["database"]["password"];
        config.databaseName = l_jConfig["database"]["databaseName"];

        SPDLOG_LOGGER_INFO(SQL_LOG, "start connect mysql Server...");
        
        // 使用 SessionPool 单例进行初始化
        if (sqlApi::SessionPool::instance().init(config) != 0) {
            return -1;
        }

        SPDLOG_LOGGER_INFO(SQL_LOG, "init session pool success!");
        return 0;
    }

    // 获取观影视图
    int iGetWatchListFullView(std::string &p_strWatchListOutJson, int l_iPage, int l_iPageSize)
    {
        SPDLOG_LOGGER_TRACE(SQL_LOG,"Start");

        // 1. ***新的连接获取方式***
        std::unique_ptr<sqlApi::SessionGuard> sessionGuard;
        try {
            // 借用一个连接，如果池中没有且达到上限，这里可能会阻塞或抛出超时异常
            sessionGuard = sqlApi::SessionPool::instance().acquire();
        } catch (const std::exception& e) {
            SPDLOG_LOGGER_ERROR(SQL_LOG, "Failed to acquire session: {}", e.what());
            return JC_ERR_CODE_SQL_CONNECTION; // 返回连接错误
        }

        // 求总页数
        if (sessionGuard == nullptr)
            return JC_ERR_CODE_SQL_CONNECTION;

        int l_iOffset = 0;
        if (l_iPage < 1)
        {
            l_iPage = 1;
        }
        l_iOffset = (l_iPage - 1) * l_iPageSize;

        // 执行sql语句查询
        try
        {
            int l_iTotalCount = 0;
            int l_iTotalPages = 0;
            // 获取总数据数量
            mysqlx::SqlResult l_SqlCountResult = sessionGuard->get().sql(std::string("SELECT COUNT(*) from ") + MYSQL_TABLE_NAME_WATCH_LIST_FULL_VIEW).execute();

            mysqlx::Row l_Row = l_SqlCountResult.fetchOne();
            l_iTotalCount = int(l_Row[0]);
            l_iTotalPages = (l_iTotalCount + l_iPageSize - 1) / l_iPageSize;

            // 根据实际数据量调整offset
            if (l_iPage > l_iTotalPages)
            {
                l_iPage = l_iTotalPages;
                l_iOffset = (l_iPage - 1) * l_iPageSize;
            }
        

            // 获取实际分页数据
            std::stringstream l_ssSql;
            l_ssSql << "SELECT id,name,eng_name,tags,type,rate,status,year,"
                    <<"DATE_FORMAT(start_time, '%Y-%m-%d') as start_time,"
                    <<"DATE_FORMAT(finish_time, '%Y-%m-%d') as finish_time from "
                    << MYSQL_TABLE_NAME_WATCH_LIST_FULL_VIEW
                    << " LIMIT "
                    << l_iPageSize
                    << " OFFSET "
                    << l_iOffset;
            
            mysqlx::SqlResult l_SqlResult = sessionGuard->get().sql(l_ssSql.str()).execute();

            mysqlx::Row l_CurRow;
            json l_jResult;
            std::vector<json> l_vecJsonResult;
            l_vecJsonResult.reserve(l_SqlResult.count());
            while ((l_CurRow = l_SqlResult.fetchOne())) // 遍历所有结果，封装成json
            {
                json l_jItem;

                l_jItem["id"] = int(l_CurRow[0]);
                l_jItem["name"] = string(l_CurRow[1].isNull() ? "null" : l_CurRow[1]);
                l_jItem["eng_name"] = string(l_CurRow[2].isNull() ? "null" : l_CurRow[2]);
                
                // tags
                {
                    l_jItem["tags"] = json::array();
                    if (!l_CurRow[3].isNull())
                    {
                        string l_strTags = string(l_CurRow[3]);
                        vector<string> l_vecStrTags = commonUse::vecStrGetArraySplitBy(l_strTags.c_str(), ',');
                        l_jItem["tags"] = l_vecStrTags;
                    }
                }
                
                
                l_jItem["type"] = string(l_CurRow[4].isNull() ? "null" : l_CurRow[4]);
                l_jItem["rate"] = l_CurRow[5].isNull() ? -1 : int(l_CurRow[5]);
                l_jItem["status"] = string(l_CurRow[6].isNull() ? "null" : l_CurRow[6]);
                l_jItem["year"] = l_CurRow[7].isNull() ? 0 : l_CurRow[7].get<int>();
                l_jItem["start_time"] = string(l_CurRow[8].isNull() ? "null" : l_CurRow[8].get<std::string>());
                l_jItem["finish_time"] = string(l_CurRow[9].isNull() ? "null" : l_CurRow[9].get<std::string>());
                l_vecJsonResult.push_back(l_jItem);

            }

            l_jResult["data"] = l_vecJsonResult;
            l_jResult["totalPage"] = l_iTotalPages;
            l_jResult["page"] = l_iPage;
            
            p_strWatchListOutJson = l_jResult.dump();
            if(p_strWatchListOutJson.length() < 100)
                SPDLOG_LOGGER_INFO(SQL_LOG,"get result:{}",p_strWatchListOutJson);
            else
                SPDLOG_LOGGER_INFO(SQL_LOG,"get result:{}......",p_strWatchListOutJson.substr(0,100));
            
        }
        catch (const mysqlx::Error &err)
        {
            SPDLOG_LOGGER_ERROR(SQL_LOG,"MySQL SQL Error:{}",err.what());
            
        }
        catch (const std::exception &e)
        {
            SPDLOG_LOGGER_ERROR(SQL_LOG,"Std Error:{}",e.what());
            
        }
        
        return JC_ERR_CODE_OK;
    }



    // ai写的
    // 初始化连接池
    int SessionPool::init(const DbConfig& config) {
        m_config = config;
        SPDLOG_LOGGER_INFO(SQL_LOG, "SessionPool: Initializing with min size {}", MIN_POOL_SIZE);
        
        try {
            // 预先创建最小数量的连接
            for (size_t i = 0; i < MIN_POOL_SIZE; ++i) {
                mysqlx::Session* sess = createSession();
                m_idleSessions.push_back(sess);
            }
            SPDLOG_LOGGER_INFO(SQL_LOG, "SessionPool: {} sessions successfully created.", MIN_POOL_SIZE);
            return 0;
        } catch (const mysqlx::Error &e) {
            SPDLOG_LOGGER_ERROR(SQL_LOG, "SessionPool initialization failed: {}", e.what());
            return -1;
        }
    }

    // ai写的
    // 线程安全地获取连接 (借用)
    std::unique_ptr<SessionGuard> SessionPool::acquire() {
        std::unique_lock<std::mutex> lock(m_mutex);
        
        // 1. 检查是否有空闲连接
        if (!m_idleSessions.empty()) {
            mysqlx::Session* session = m_idleSessions.back();
            m_idleSessions.pop_back();
            SPDLOG_LOGGER_DEBUG(SQL_LOG,"Get Session: {:p}",(void*)session);
            // 可以在此处添加连接健康检查，例如 session->ping()
            return std::make_unique<SessionGuard>(session, this);
        }
        
        // 2. 如果没有空闲连接，检查是否达到最大限制
        if (m_idleSessions.size() < MAX_POOL_SIZE) {
            // 还没满，创建新连接 (在锁内，但这通常很快)
            try {
                mysqlx::Session* session = createSession();
                SPDLOG_LOGGER_INFO(SQL_LOG, "SessionPool: Created new session (current size: {}).", m_idleSessions.size() + 1);
                return std::make_unique<SessionGuard>(session, this);
            } catch (const mysqlx::Error &e) {
                SPDLOG_LOGGER_ERROR(SQL_LOG, "SessionPool: Failed to create new session: {}", e.what());
                throw; // 重新抛出异常，请求失败
            }
        }
        
        // 3. 达到最大限制，阻塞等待空闲连接
        SPDLOG_LOGGER_TRACE(SQL_LOG, "SessionPool: Pool full, waiting for connection...");
        // 等待条件变量，最多等待 5 秒（防止无限阻塞）
        if (m_cond.wait_for(lock, std::chrono::seconds(5), [this]{ return !m_idleSessions.empty(); })) {
            // 被唤醒且有连接
            mysqlx::Session* session = m_idleSessions.back();
            m_idleSessions.pop_back();
            return std::make_unique<SessionGuard>(session, this);
        } else {
            // 等待超时
            SPDLOG_LOGGER_ERROR(SQL_LOG, "SessionPool: Acquire timed out (Pool size: {}).", MAX_POOL_SIZE);
            throw std::runtime_error("Database connection pool timeout");
        }
    }

    // ai写的
    // 线程安全地释放连接 (归还)
    void SessionPool::release(mysqlx::Session* session) {
        if (!session) return;

        std::unique_lock<std::mutex> lock(m_mutex);
        // 归还前，可以检查连接是否仍然健康。如果不健康，delete session 并记录日志。
        
        m_idleSessions.push_back(session);
        SPDLOG_LOGGER_DEBUG(SQL_LOG, "Released Session: {:p}",(void*)session);
        
        // 通知所有等待的线程，有新的连接可用
        m_cond.notify_one(); 
    }

    // ai写的
    // 析构函数：清理所有连接
    SessionPool::~SessionPool() {
        for (mysqlx::Session* session : m_idleSessions) {
            delete session;
        }
        m_idleSessions.clear();
    }

} // namespace sqlApi