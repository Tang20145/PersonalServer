#include "xdevapi.h"
#include "json.hpp"
#include <vector>
#include <string>
using json = nlohmann::json;

int main()
{

    return;
}

json getPaginatedDataAsJson(const string& tableName, 
                           int page = 1, 
                           int perPage = 10,
                           const string& orderBy = "id") {
    try {
        // 1. 连接MySQL
        mysqlx::Session sess("localhost", 3306, "root", "root");
        Schema schema = sess.getSchema("your_database");
        Table table = schema.getTable(tableName);
        // 2. 计算分页偏移量
        int offset = (page - 1) * perPage;
        // 3. 获取表结构元数据
        auto columns = table.getColumns();
        vector<string> fieldNames;
        vector<Column::Type> fieldTypes;
        for (const auto& col : columns) {
            fieldNames.push_back(col.getColumnName());
            fieldTypes.push_back(col.getType());
        }
        // 4. 执行分页查询
        RowResult result = table.select("*")
                          .orderBy(orderBy)
                          .limit(perPage)
                          .offset(offset)
                          .execute();
        // 5. 构建JSON结果
        json j;
        json header;
        // 5.1 添加字段名作为第一行
        for (size_t i = 0; i < fieldNames.size(); ++i) {
            header[fieldNames[i]] = fieldTypes[i] == Column::Type::DATE ? 
                                   "DATE" : mysqlx_to_string(fieldTypes[i]);
        }
        j["columns"] = header;
        // 5.2 添加数据行
        json data = json::array();
        for (Row row : result) {
            json rowData;
            for (size_t i = 0; i < row.colCount(); ++i) {
                if (row[i].isNull()) {
                    rowData[fieldNames[i]] = nullptr;
                    continue;
                }
                switch (fieldTypes[i]) {
                    case Column::Type::INT:
                    case Column::Type::SMALLINT:
                    case Column::Type::BIGINT:
                        rowData[fieldNames[i]] = row[i];
                        break;
                    case Column::Type::VARCHAR:
                    case Column::Type::CHAR:
                    case Column::Type::TEXT:
                        rowData[fieldNames[i]] = string(row[i]);
                        break;
                    case Column::Type::DATE: {
                        // 格式化为YYYY-MM-DD
                        time_t t = row[i];
                        tm* tmPtr = gmtime(&t);
                        char buffer[11];
                        strftime(buffer, sizeof(buffer), "%Y-%m-%d", tmPtr);
                        rowData[fieldNames[i]] = string(buffer);
                        break;
                    }
                    case Column::Type::DECIMAL:
                        rowData[fieldNames[i]] = stod(string(row[i]));
                        break;
                    case Column::Type::DOUBLE:
                    case Column::Type::FLOAT:
                        rowData[fieldNames[i]] = row[i];
                        break;
                    default:
                        rowData[fieldNames[i]] = string(row[i]);
                }
            }
            data.push_back(rowData);
        }
        // 6. 添加分页信息
        int totalCount = table.count();
        j["data"] = data;
        j["pagination"] = {
            {"total", totalCount},
            {"per_page", perPage},
            {"current_page", page},
            {"total_pages", (totalCount + perPage - 1) / perPage}
        };
        sess.close();
        return j;
    } catch (const mysqlx::Error &err) {
        return json{{"error", err.what()}};
    } catch (const exception &e) {
        return json{{"error", e.what()}};
    }
}
// 辅助函数：将MySQL类型转换为字符串表示
string mysqlx_to_string(Column::Type type) {
    static const map<Column::Type, string> typeMap = {
        {Column::Type::INT, "INT"},
        {Column::Type::VARCHAR, "VARCHAR"},
        {Column::Type::CHAR, "CHAR"},
        {Column::Type::DATE, "DATE"},
        {Column::Type::DECIMAL, "DECIMAL"},
        {Column::Type::DOUBLE, "DOUBLE"},
        {Column::Type::FLOAT, "FLOAT"},
        {Column::Type::TEXT, "TEXT"},
        {Column::Type::BIGINT, "BIGINT"},
        {Column::Type::SMALLINT, "SMALLINT"}
    };
    return typeMap.count(type) ? typeMap.at(type) : "UNKNOWN";
}