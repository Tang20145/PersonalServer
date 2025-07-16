#include "sqlApi.h"
#include "string.h"
#include <fstream>
#include <iostream>
#include <vector>

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

    mysqlx::Session *g_nSess;

    int init()
    {
        // 读取json配置文件
        std::ifstream l_fConfig("config.json");
        json l_jConfig = json::parse(l_fConfig);

        {
            string host = l_jConfig["database"]["host"];
            string username = l_jConfig["database"]["username"];
            string password = l_jConfig["database"]["password"];
            string databaseName = l_jConfig["database"]["databaseName"];
            // 初始化会话
            g_nSess = new mysqlx::Session(host, 33060, username, password, databaseName);
        }
    }

    string getWatchListFullViewJsonString()
    {
        // 查询视图，需要将DATE格式化输出，不然会返回二进制类型RAW，无法转换为Json
        string l_sSql = "SELECT id,name,eng_name,tags,type,rate,status,DATE_FORMAT(start_time, \"%Y-%m-%d\") AS formatted_start_time,DATE_FORMAT(finish_time, \"%Y-%m-%d\") AS formatted_finish_time,comment,link FROM WatchListFullView";
        mysqlx::SqlResult l_rWatchListFullView = g_nSess->sql(l_sSql).execute();

        // json 结果
        json l_jWatchListFullView;

        // 对每行处理，完成json结果写入
        for (mysqlx::Row row : l_rWatchListFullView)
        {
            json l_jItem;

            // 提取字段
            l_jItem["id"] = row[0].get<int>();
            l_jItem["name"] = row[1];
            l_jItem["eng_name"] = row[2].isNull() ? "" : row[2];
            // l_jItem["tags"] = row[3];
            l_jItem["type"] = row[4];
            // nullptr 会为什么？
            if (!row[5].isNull())
                l_jItem["rate"] = row[5].get<int>();
            l_jItem["status"] = row[6];

            if (!row[7].isNull())
                l_jItem["start_time"] = row[7];
            if (!row[8].isNull())
                l_jItem["finish_time"] = row[8];
            else
                l_jItem["finish_time"].clear();
            if (!row[9].isNull())
                l_jItem["comment"] = row[9];
            if (!row[10].isNull())
                l_jItem["link"] = row[10];
            else
                l_jItem["link"].clear();
            // 处理以逗号 ',' 为分隔的标签字段，转为vector
            {
                mysqlx::string l_sTagStr = row[3];
                vector<mysqlx::string> l_vTags;
                size_t l_ulPos;
                while ((l_ulPos = l_sTagStr.find(',')) != mysqlx::string::npos) // 当找得到分隔符，l_ulPos为分隔符的索引
                {
                    // 取出此段字符并删掉
                    l_vTags.push_back(l_sTagStr.substr(0, l_ulPos));
                    l_sTagStr.erase(0, l_ulPos + 1);
                }

                if (!l_sTagStr.empty())
                    l_vTags.push_back(l_sTagStr);
                l_jItem["tags"] = l_vTags;
            }

            // 加入此行
            l_jWatchListFullView.push_back(l_jItem);
        }

        return l_jWatchListFullView.dump();
    }

    string getSqlQueryJsonString(string l_sSql)
    {
        // 查询
        mysqlx::SqlResult l_rSqlResult = g_nSess->sql(l_sSql).execute();

        // 获取列名
        const mysqlx::Columns &l_Columns = l_rSqlResult.getColumns();
        std::vector<string> l_aColumnNames;
        auto l_itColumns = l_Columns.begin();
        while (l_itColumns != l_Columns.end())
        {
            l_aColumnNames.push_back(l_itColumns->getColumnLabel());
            // 测试列名输出
            // std::cout << l_aColumnNames.back().c_str()<< ",";
            l_itColumns++;
        }

        // 结果
        json l_jResDataJson;

        // 每行
        for (mysqlx::Row row : l_rSqlResult)
        {
            // 每行的数据json对象
            json l_jItem;

            // 每列（每个字段
            for (int i = 0; i < l_aColumnNames.size(); i++)
            {
                string l_sColumnName = l_aColumnNames[i];
                const mysqlx::Value l_Val = row[i];

                // 字段为空
                if (l_Val.isNull())
                {
                    l_jItem[l_sColumnName] = nullptr;
                }
                else // 字段不为空
                {
                    // 字段是逗号分隔的标签类型
                    if (l_sColumnName.length() > strlen("_tags") && l_sColumnName.substr(l_sColumnName.length() - strlen("_tags"), strlen("_tags")) == "_tags") // 如果是标签类型
                    {
                        string l_sTagStr = l_Val.get<string>();
                        vector<string> l_vTags;
                        int l_iPos;
                        while ((l_iPos = l_sTagStr.find(',')) != string::npos) // 当找得到分隔符，l_iPos为分隔符的索引
                        {
                            // 取出此段字符并删掉
                            l_vTags.push_back(l_sTagStr.substr(0, l_iPos));
                            l_sTagStr.erase(0, l_iPos + 1);
                        }

                        if (!l_sTagStr.empty())
                            l_vTags.push_back(l_sTagStr);

                        // 直接返回不带_tags尾缀的
                        l_jItem[l_sColumnName.substr(0, l_sColumnName.length() - strlen("_tags"))] = l_vTags;
                    }
                    else // 字段为正常单个值
                    {
                        switch (l_Val.getType())
                        {
                        case mysqlx::Value::Type::INT64:
                            l_jItem[l_sColumnName] = l_Val.get<int64_t>();
                            break;
                        case mysqlx::Value::Type::UINT64:
                            l_jItem[l_sColumnName] = l_Val.get<u_int64_t>();
                            break;
                        default:
                            l_jItem[l_sColumnName] = l_Val.get<string>();
                            break;
                        }
                    }
                }
            }
            l_jResDataJson.push_back(l_jItem);
        }
        json l_jResJson;
        l_jResJson["recordsTotal"] = l_jResDataJson.size();
        l_jResJson["data"] = l_jResDataJson;
        return l_jResJson.dump();
    }

    std::string getSqlQueryJsonString(std::unordered_map<std::string,std::string> l_mIn)
    {
        // 获取查询语句
        string l_sSql = l_mIn["sql"];
        

        // 查询
        mysqlx::SqlResult l_rSqlResult = g_nSess->sql(l_sSql).execute();

        // 获取列名
        const mysqlx::Columns &l_Columns = l_rSqlResult.getColumns();
        std::vector<string> l_aColumnNames;
        auto l_itColumns = l_Columns.begin();
        while (l_itColumns != l_Columns.end())
        {
            l_aColumnNames.push_back(l_itColumns->getColumnLabel());
            // 测试列名输出
            // std::cout << l_aColumnNames.back().c_str()<< ",";
            l_itColumns++;
        }

        // 结果
        json l_jResDataJson;

        // 每行
        for (mysqlx::Row row : l_rSqlResult)
        {
            // 每行的数据json对象
            json l_jItem;

            // 每列（每个字段
            for (int i = 0; i < l_aColumnNames.size(); i++)
            {
                string l_sColumnName = l_aColumnNames[i];
                const mysqlx::Value l_Val = row[i];

                // 字段为空
                if (l_Val.isNull())
                {
                    l_jItem[l_sColumnName] = nullptr;
                }
                else // 字段不为空
                {
                    // 字段是逗号分隔的标签类型
                    if (l_sColumnName.length() > strlen("_tags") && l_sColumnName.substr(l_sColumnName.length() - strlen("_tags"), strlen("_tags")) == "_tags") // 如果是标签类型
                    {
                        string l_sTagStr = l_Val.get<string>();
                        vector<string> l_vTags;
                        int l_iPos;
                        while ((l_iPos = l_sTagStr.find(',')) != string::npos) // 当找得到分隔符，l_iPos为分隔符的索引
                        {
                            // 取出此段字符并删掉
                            l_vTags.push_back(l_sTagStr.substr(0, l_iPos));
                            l_sTagStr.erase(0, l_iPos + 1);
                        }

                        if (!l_sTagStr.empty())
                            l_vTags.push_back(l_sTagStr);

                        // 直接返回不带_tags尾缀的
                        l_jItem[l_sColumnName.substr(0, l_sColumnName.length() - strlen("_tags"))] = l_vTags;
                    }
                    else // 字段为正常单个值
                    {
                        switch (l_Val.getType())
                        {
                        case mysqlx::Value::Type::INT64:
                            l_jItem[l_sColumnName] = l_Val.get<int64_t>();
                            break;
                        case mysqlx::Value::Type::UINT64:
                            l_jItem[l_sColumnName] = l_Val.get<u_int64_t>();
                            break;
                        default:
                            l_jItem[l_sColumnName] = l_Val.get<string>();
                            break;
                        }
                    }
                }
            }
            l_jResDataJson.push_back(l_jItem);
        }
        json l_jResJson;
        l_jResJson["recordsTotal"] = l_jResDataJson.size();
        l_jResJson["data"] = l_jResDataJson;
        return l_jResJson.dump();
    }
} // namespace sqlApi