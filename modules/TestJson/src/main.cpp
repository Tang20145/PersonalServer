#include "xdevapi.h"
#include "json.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace std;
using json = nlohmann::json;

int main()
{

    // 连接会话
    mysqlx::Session l_nSess("localhost", 33060, "root", "root", "personalServerDB");
    // 查询视图
    mysqlx::SqlResult l_rWatchListFullView = l_nSess.sql("SELECT * FROM WatchListFullView").execute();

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
        l_jItem["rate"] = row[5].isNull() ? nullptr : json(row[5].get<int>());
        l_jItem["status"] = row[6];
        mysqlx::Value test = row[7];
        string testStr = test.get_string();
        
        
        // l_jItem["start_time"] = row[7].isNull() ? "null" : row[7].get<string>();
        // l_jItem["finish_time"] = row[8].isNull() ? nullptr : row[8].get<string>();
        // l_jItem["comment"] = row[9].isNull() ? nullptr : row[9];
        // l_jItem["link"] = row[10].isNull() ? nullptr : row[10];

        

        // 处理以逗号 ',' 为分隔的标签，转为vector
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

        // 加入此行
        l_jWatchListFullView.push_back(l_jItem);
    }

    // 格式化输出
    cout << l_jWatchListFullView.dump(4) << endl;

    return 0;
}