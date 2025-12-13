#ifndef SESSION_POOL_H_
#define SESSION_POOL_H_

// 数据库连接池
#include "xdevapi.h"
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace sqlApi
{
    // 存储数据库连接配置
    struct DbConfig {
        std::string host;
        int port = 33060;
        std::string user;
        std::string password;
        std::string databaseName;
    };

    // Session 管理类：用于自动归还连接，声明即构造，析构即回收
    class SessionGuard {
    private:
        mysqlx::Session* m_session;
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

    // 连接池类
    class SessionPool
    {
        private:
        DbConfig m_stDbonfig;
        std::vector<mysqlx::Session*> m_vecIdleSessions; // 空闲连接池
        std::mutex m_mtx;// 锁
        std::condition_variable m_cond; // 用于等待连接释放
        
        const size_t MAX_POOL_SIZE = 10; // 最大连接数限制
        const size_t MIN_POOL_SIZE = 3;  // 最小连接数（初始化时创建）
    }










}

#endif