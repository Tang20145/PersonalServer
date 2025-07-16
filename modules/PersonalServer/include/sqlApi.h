#ifndef SQLAPI_H_
#define SQLAPI_H_

#include "xdevapi.h"
#include "json.hpp"
#include <unordered_map>
#include <string>
using json = nlohmann::json;

// sql使用会话
extern mysqlx::Session* g_nSess;

// 使用命名空间防止冲突
namespace sqlApi
{
    // 初始化连接会话
    int init();

    // 获取观影列表视图
    std::string getWatchListFullViewJsonString();

    // 通用获取sql查询结果函数，字段名以_tags为尾缀即为数组标签
    std::string getSqlQueryJsonString(std::string l_sSql);

    // 通用获取sql查询结果函数，引入page pageSize等等
    std::string getSqlQueryJsonString(std::unordered_map<std::string,std::string> l_mIn);

}// namespace sqlApi
#endif