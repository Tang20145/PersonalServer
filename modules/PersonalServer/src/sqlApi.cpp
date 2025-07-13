#include "sqlApi.h"


// 初始化会话
mysqlx::Session g_nSess("localhost", 33060, "root", "root", "personalServerDB");

string getWatchListFullViewJsonString()
{
    // 查询视图，需要将DATE格式化输出，不然会返回二进制类型RAW，无法转换为Json
    string l_sSql = "SELECT id,name,eng_name,tags,type,rate,status,DATE_FORMAT(start_time, \"%Y-%m-%d\") AS formatted_start_time,DATE_FORMAT(finish_time, \"%Y-%m-%d\") AS formatted_finish_time,comment,link FROM WatchListFullView";
    mysqlx::SqlResult l_rWatchListFullView = g_nSess.sql(l_sSql).execute();

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
        if(!row[5].isNull())
        l_jItem["rate"] = row[5].get<int>();
        l_jItem["status"] = row[6];

        if (!row[7].isNull())
            l_jItem["start_time"] = row[7];
        if (!row[8].isNull())
            l_jItem["finish_time"] = row[8];
        if (!row[9].isNull())
            l_jItem["comment"] = row[9];
        if (!row[10].isNull())
            l_jItem["link"] = row[10];

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