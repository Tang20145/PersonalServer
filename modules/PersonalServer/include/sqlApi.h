#ifndef SQLAPI_H_
#define SQLAPI_H_

#include "xdevapi.h"
#include "json.hpp"
#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
using json = nlohmann::json;

#define SQL_TABLE_NAME "tableName"
#define SQL_PAGE "page"
#define SQL_PAGE_SIZE "pageSize"
#define SQL_LIMIT_DEFAULT 20 // 默认每页20项

#define MYSQL_DATABASE_NAME_PERSONAL_SERVER_DB "personalServerDB"
// 数据表
#define MYSQL_TABLE_NAME_WATCH_LIST_FULL_VIEW "WatchListFullView"
// #define MYSQL_TABLE_WATCH_LIST_FULL_VIEW_COL_ID "id"


// sql使用会话
// extern mysqlx::Session* g_nSess;// 移除全局会话

// 使用命名空间防止冲突
namespace sqlApi
{
    // 会话连接池 Start ------------------------------------------------

    // 连接配置
    struct DbConfig {
        std::string host;
        int port;
        std::string user;
        std::string password;
        std::string databaseName;
    };

    // 核心类：既负责记录时间，又负责在析构时自动归还连接
    class SessionItem {
    public:
        // 构造函数用于从池中取出
        SessionItem(mysqlx::Session* s) : m_session(s), m_lastActiveTime(std::chrono::steady_clock::now()) {}
        
        // 析构函数：核心逻辑，对象离开作用域时自动把 session 还给池子
        ~SessionItem();

        // 提供给业务使用的接口
        mysqlx::Session& get() { return *m_session; }
        std::chrono::steady_clock::time_point getTime() { return m_lastActiveTime; }

    private:
        mysqlx::Session* m_session;
        std::chrono::steady_clock::time_point m_lastActiveTime;
    };

    class SessionPool {
    public:
        static SessionPool& instance() {
            static SessionPool pool;
            return pool;
        }

        int init(const DbConfig& config);
        // 返回 unique_ptr 确保业务代码结束时能自动触发 SessionItem 的析构
        std::unique_ptr<SessionItem> acquire();
        void release(mysqlx::Session* s);

        mysqlx::Session* createNew();

    private:
        SessionPool() = default;
        DbConfig m_config;
        std::deque<mysqlx::Session*> m_idleSessions;
        std::deque<std::chrono::steady_clock::time_point> m_idleTimes; // 记录对应的入池时间
        std::mutex m_mutex;
    };

    // 会话连接池 End ------------------------------------------------

    // 初始化连接会话
    int init();

    // 分页获取观影列表视图
    int iGetWatchListFullView(std::string& p_strWatchListOutJson,int l_iPage,int l_iPageSize);

}// namespace sqlApi
#endif