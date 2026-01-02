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
        std::unique_ptr<sqlApi::SessionItem> l_ptrSessionItem;
        try {
            // 借用一个连接，如果池中没有且达到上限，这里可能会阻塞或抛出超时异常
            l_ptrSessionItem = sqlApi::SessionPool::instance().acquire();
        } catch (const std::exception& e) {
            SPDLOG_LOGGER_ERROR(SQL_LOG, "Failed to acquire session: {}", e.what());
            return JC_ERR_CODE_SQL_CONNECTION; // 返回连接错误
        }

        // 求总页数
        if (l_ptrSessionItem.get() == nullptr)
        {
            SPDLOG_LOGGER_ERROR(SQL_LOG,"acquire sql session null!");
            return JC_ERR_CODE_SQL_CONNECTION;
        }

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
            mysqlx::SqlResult l_SqlCountResult = l_ptrSessionItem.get()->get().sql(std::string("SELECT COUNT(*) from ") + MYSQL_TABLE_NAME_WATCH_LIST_FULL_VIEW).execute();

            mysqlx::Row l_Row = l_SqlCountResult.fetchOne();
            l_iTotalCount = int(l_Row[0]);
            l_iTotalPages = (l_iTotalCount + l_iPageSize - 1) / l_iPageSize;

            // 根据实际数据量调整offset
            if (l_iPage > l_iTotalPages)
            {
                l_iPage = l_iTotalPages;
                l_iOffset = (l_iPage - 1) * l_iPageSize;
            }

            while(l_SqlCountResult.fetchOne())
            {
                SPDLOG_LOGGER_TRACE(SQL_LOG,"SELECT COUNT(*) from {} , fetched one more row",MYSQL_TABLE_NAME_WATCH_LIST_FULL_VIEW);
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
            
            mysqlx::SqlResult l_SqlResult = l_ptrSessionItem.get()->get().sql(l_ssSql.str()).execute();

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
            return JC_ERR_CODE_SQL_ERR_UNKNOWN;
        }
        catch (const std::exception &e)
        {
            SPDLOG_LOGGER_ERROR(SQL_LOG,"Std Error:{}",e.what());
            return JC_ERR_CODE_ERR;
        }
        
        return JC_ERR_CODE_OK;
    }

    // --- SessionItem 析构时自动归还 ---
    SessionItem::~SessionItem() {
        if (m_session) {
            SessionPool::instance().release(m_session);
        }
    }

    // --- 池管理逻辑 ---

    int SessionPool::init(const DbConfig& config) {
        m_config = config;
        return 0;
    }

    mysqlx::Session* SessionPool::createNew() {
        mysqlx::Session* p = new mysqlx::Session(m_config.host, m_config.port, m_config.user, m_config.password, m_config.databaseName);
        SPDLOG_LOGGER_TRACE(SQL_LOG,"new session created {:p}",(void*)p);
        return p;
    }

    std::unique_ptr<SessionItem> SessionPool::acquire() {
        SPDLOG_LOGGER_TRACE(SQL_LOG,"Start");
        std::lock_guard<std::mutex> lock(m_mutex);
        SPDLOG_LOGGER_TRACE(SQL_LOG,"Get session mutex");
        auto now = std::chrono::steady_clock::now();

        while (!m_idleSessions.empty()) {
            mysqlx::Session* s = m_idleSessions.front();
            auto lastTime = m_idleTimes.front();
            
            m_idleSessions.pop_front();
            m_idleTimes.pop_front();

            // 核心：60秒超时逻辑
            if (std::chrono::duration_cast<std::chrono::seconds>(now - lastTime).count() > 60) {
                try { s->close(); } catch(...) {}
                SPDLOG_LOGGER_TRACE(SQL_LOG,"delete session {:p}",(void*)s);
                delete s;
                // 过期了就新建一个直接给用户
                return std::make_unique<SessionItem>(createNew());
            }
            // 没过期，封装返回
            return std::make_unique<SessionItem>(s);
        }

        // 池子空，新建
        return std::make_unique<SessionItem>(createNew());
    }

    void SessionPool::release(mysqlx::Session* s) {
        if (!s) return;
        std::lock_guard<std::mutex> lock(m_mutex);
        m_idleSessions.push_back(s);
        m_idleTimes.push_back(std::chrono::steady_clock::now());
    }

} // namespace sqlApi