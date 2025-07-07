// mysqlx是基于X协议的
#include "xdevapi.h"

int main()
{
    mysqlx::Session sess("host", 3306, "root", "root", "personalServerDB");
    
    return 0;
}