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
    // ai写的
    // 存储数据库连接配置
    struct DbConfig {
        std::string host;
        int port = 33060;
        std::string user;
        std::string password;
        std::string databaseName;
    };

    // ai写的
    // Session 管理类：用于自动归还连接
    class SessionGuard {
    private:
        mysqlx::Session* m_session;
        // 需要一个指向 SessionPool 的指针，以便在析构时调用 Release
        class SessionPool* m_pool; 

    public:
        // 构造函数：从 Pool 借用 Session
        SessionGuard(mysqlx::Session* session, SessionPool* pool) 
            : m_session(session), m_pool(pool) {}

        // 析构函数：保证在 SessionGuard 生命周期结束时，连接被归还 (RAII)
        ~SessionGuard(); 

        // 提供对底层 Session 的访问
        mysqlx::Session& get() { return *m_session; }
        mysqlx::Session* operator->() { return m_session; }
    };

    // ai写的
    class SessionPool {
    private:
        DbConfig m_config;
        std::vector<mysqlx::Session*> m_idleSessions; // 空闲连接池
        std::mutex m_mutex;
        std::condition_variable m_cond; // 用于等待连接释放
        
        const size_t MAX_POOL_SIZE = 10; // 最大连接数限制
        const size_t MIN_POOL_SIZE = 3;  // 最小连接数（初始化时创建）

        // 内部方法：创建新 Session
        mysqlx::Session* createSession() {
            return new mysqlx::Session(
                m_config.host, 
                m_config.port, 
                m_config.user, 
                m_config.password, 
                m_config.databaseName
            );
        }

    public:
        // 单例模式，保证全局只有一个 Pool 实例
        static SessionPool& instance() {
            static SessionPool instance;
            return instance;
        }

        // 禁用拷贝和赋值
        SessionPool(const SessionPool&) = delete;
        void operator=(const SessionPool&) = delete;
        
        // 构造函数私有化
        SessionPool() = default; 

        // 初始化配置和连接
        int init(const DbConfig& config);

        // 核心方法：获取连接 (线程安全)
        std::unique_ptr<SessionGuard> acquire();

        // 核心方法：释放连接 (线程安全)
        void release(mysqlx::Session* session);

        // 析构函数：清理所有连接
        ~SessionPool();
    };
    
    // ai写的
    // 实现 SessionGuard 析构函数
    inline SessionGuard::~SessionGuard() {
        if (m_session && m_pool) {
            m_pool->release(m_session);
        }
    }

    // 会话连接池 End ------------------------------------------------

    // 初始化连接会话
    int init();

    // 分页获取观影列表视图
    int iGetWatchListFullView(std::string& p_strWatchListOutJson,int l_iPage,int l_iPageSize);

}// namespace sqlApi
#endif