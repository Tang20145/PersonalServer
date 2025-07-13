// mysqlx是基于X协议的
// api文档：https://dev.mysql.com/doc/dev/connector-cpp/latest/
#include "xdevapi.h"

int main()
{
    mysqlx::Session sess("localhost", 33060, "root", "root", "personalServerDB");
    
    return 0;
}