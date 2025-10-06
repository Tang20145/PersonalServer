#include "sqlApi.h"
#include "string.h"
#include <fstream>
#include <iostream>
#include <vector>
#include "commonUse.h"

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

    mysqlx::Session *g_nSess = nullptr; // 定义会话

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
            if (g_nSess != nullptr)
            {
                g_nSess->close();
                delete g_nSess;
                g_nSess = nullptr;
            }
            try
            {
                g_nSess = new mysqlx::Session(host, 33060, username, password, databaseName);
                std::cout << "init session success!\n";
            }
            catch (const mysqlx::Error &e)
            {
                std::cerr << e.what() << "\n"; // mysqlx的异常
                return -1;
            }
        }
        return 0;
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

    int iGetWatchListFullView(std::string &p_strWatchListOutJson, int l_iPage, int l_iPageSize)
    {
        // 求总页数
        if (g_nSess == nullptr)
            return -1;

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
            mysqlx::SqlResult l_SqlCountResult = g_nSess->sql(std::string("SELECT COUNT(*) from ") + MYSQL_TABLE_NAME_WATCH_LIST_FULL_VIEW).execute();

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
            l_ssSql << "SELECT id,name,eng_name,tags,type,rate,status,"
                    <<"DATE_FORMAT(start_time, '%Y-%m-%d') as start_time,"
                    <<"DATE_FORMAT(finish_time, '%Y-%m-%d') as finish_time from "
                    << MYSQL_TABLE_NAME_WATCH_LIST_FULL_VIEW
                    << " LIMIT "
                    << l_iPageSize
                    << " OFFSET "
                    << l_iOffset;

            mysqlx::SqlResult l_SqlResult = g_nSess->sql(l_ssSql.str()).execute();

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
                        // for(auto s : l_vecStrTags){
                        //     l_jItem["tags"].push_back(s);
                        // }
                    }
                }
                l_jItem["type"] = string(l_CurRow[4].isNull() ? "null" : l_CurRow[4]);
                l_jItem["rate"] = l_CurRow[5].isNull() ? -1 : int(l_CurRow[5]);
                l_jItem["status"] = string(l_CurRow[6].isNull() ? "null" : l_CurRow[6]);
                l_jItem["start_time"] = string(l_CurRow[7].isNull() ? "null" : l_CurRow[7]);
                l_jItem["finish_time"] = string(l_CurRow[8].isNull() ? "null" : l_CurRow[8]);

                l_vecJsonResult.push_back(l_jItem);
            }
            l_jResult["data"] = l_vecJsonResult;
            l_jResult["totalPage"] = l_iTotalPages;
            l_jResult["page"] = l_iPage;
            // std::cout << "sql result : " << l_jResult.dump() << std::endl;
            p_strWatchListOutJson = l_jResult.dump();
            
        }
        catch (const mysqlx::Error &err)
        {
            std::cerr << "MySQL SQL Error: " << err.what() << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        // // 读取collection对象，弃用
        // try
        // {
        //     mysqlx::Schema l_scPersonalServerDB = g_nSess->getSchema(MYSQL_DATABASE_NAME_PERSONAL_SERVER_DB);
        //     mysqlx::Collection l_coWatchListFullView = l_scPersonalServerDB.getCollection(MYSQL_TABLE_NAME_WATCH_LIST_FULL_VIEW);
        //     int l_iCount = l_coWatchListFullView.count();// 数据总数
        //     mysqlx::DocResult l_DocResult = l_coWatchListFullView.find().limit(l_iPageSize).offset(l_iOffset).execute();
        //     mysqlx::DbDoc l_DbDoc;
        //     while(l_DbDoc=l_DocResult.fetchOne())
        //     {
        //     }
        // }
        // catch(const mysqlx::Error& err)
        // {
        //     std::cerr << err.what() << '\n';
        // }
        // catch(const std::exception& e)
        // {
        //     std::cerr << e.what() << '\n';
        // }
        
        return 0;
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

    // 通用查询
    // 暂不实现关键词查找
    std::string getSqlQueryJsonString(std::unordered_map<std::string, std::string> &l_mIn)
    {
        // 获取查询语句
        if (l_mIn.find("sql") == l_mIn.end() || l_mIn.find("countSql") == l_mIn.end())
        {
            printf("sql is empty\n");
            return "";
        }
        string l_sSql = l_mIn["sql"];
        string l_sCountSql = l_mIn["countSql"];
        int l_iCount = g_nSess->sql(l_sCountSql).execute().fetchOne()[0].get<int>();
        int l_iFilterCount = l_iCount;

        // 组装分页查询语句
        {
            // 默认分页参数，防止返回结果过多
            int l_iPage = 1;
            int l_iPageSize = 10;
            int l_iOffset = 0;
            string l_sKeyword;
            string l_sSortField;
            string l_sSortDirection = "asc"; // 默认增

            // 排序
            if (l_mIn.find("sortField") != l_mIn.end())
            {
                // 获取用于排序的字段
                l_sSql += " ORDER BY " + l_mIn["sortField"];
            }
            if (l_mIn.find("sortDirection") != l_mIn.end())
            {
                // 排序方向
                l_sSortDirection = l_mIn["sortDirection"];
            }
            l_sSql += " " + l_sSortDirection;

            // 分页
            if (l_mIn.find("pageSize") != l_mIn.end())
            {
                // 获取每页大小
                l_iPageSize = atoi(l_mIn["pageSize"].c_str());
            }
            if (l_mIn.find("page") != l_mIn.end())
            {
                // 获取页面
                l_iPage = atoi(l_mIn["page"].c_str());
                // 获取偏移
                l_iOffset = (l_iPage - 1) * l_iPageSize;
            }
            l_sSql += " LIMIT " + to_string(l_iOffset) + "," + to_string(l_iPageSize);

            // 查询关键字（暂不实现
            if (l_mIn.find("keyword") != l_mIn.end())
            {
                // 获取查找关键词
                l_sKeyword = l_mIn["keyword"];
            }
        }

        printf("SQL : %s\n", l_sSql.c_str());

        // 查询
        mysqlx::SqlResult l_rSqlResult = g_nSess->sql(l_sSql).execute();
        // mysqlx::SqlResult l_rTotal = g_nSess->sql(l_sCountSql).execute().fetchOne()[0].get<int>();
        // int l_iTotal = l_rTotal.fetchOne()[0].get<int>();

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

                // 字段是逗号分隔的标签类型
                if (l_sColumnName.length() > strlen("_tags") && l_sColumnName.substr(l_sColumnName.length() - strlen("_tags"), strlen("_tags")) == "_tags") // 如果是标签类型
                {
                    vector<string> l_vTags;

                    if (!l_Val.isNull())
                    {
                        string l_sTagStr = l_Val.get<string>();
                        int l_iPos;
                        while ((l_iPos = l_sTagStr.find(',')) != string::npos) // 当找得到分隔符，l_iPos为分隔符的索引
                        {
                            // 取出此段字符并删掉
                            l_vTags.push_back(l_sTagStr.substr(0, l_iPos));
                            l_sTagStr.erase(0, l_iPos + 1);
                        }

                        if (!l_sTagStr.empty())
                            l_vTags.push_back(l_sTagStr);
                    }

                    // 直接返回不带_tags尾缀的
                    l_jItem[l_sColumnName.substr(0, l_sColumnName.length() - strlen("_tags"))] = l_vTags;
                }
                else if (l_Val.isNull()) // 字段为空
                {
                    l_jItem[l_sColumnName] = ""; // 表示空
                }
                else // 字段不为空
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
                // string gdbString = l_jItem.dump();
                // cout << "json : " << gdbString << endl;
            }
            l_jResDataJson.push_back(l_jItem);
        }
        json l_jResJson;
        l_jResJson["draw"] = atoi(l_mIn["draw"].c_str());
        l_jResJson["recordsTotal"] = l_iCount;
        l_jResJson["recordsFiltered"] = l_iCount;
        l_jResJson["data"] = l_jResDataJson;

        return l_jResJson.dump();
    }
} // namespace sqlApi