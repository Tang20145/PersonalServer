#ifndef SQLAPI_H_
#define SQLAPI_H_

#include "xdevapi.h"
#include "json.hpp"
#include <unordered_map>
#include <string>
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
extern mysqlx::Session* g_nSess;// 声明会话

// 使用命名空间防止冲突
namespace sqlApi
{
    // 初始化连接会话
    int init();

    // 获取观影列表视图
    std::string getWatchListFullViewJsonString();

    // 分页获取观影列表视图
    int iGetWatchListFullView(std::string& p_strWatchListOutJson,int l_iPage,int l_iPageSize);

    // 通用获取sql查询结果函数，字段名以_tags为尾缀即为数组标签
    std::string getSqlQueryJsonString(std::string l_sSql);

    // 通用获取sql查询结果函数，引入page pageSize等等
    std::string getSqlQueryJsonString(std::unordered_map<std::string,std::string>& l_mIn);

}// namespace sqlApi
#endif