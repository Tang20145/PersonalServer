#ifndef SQLAPI_H_
#define SQLAPI_H_

#include "xdevapi.h"
#include "json.hpp"
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

}// namespace sqlApi
#endif