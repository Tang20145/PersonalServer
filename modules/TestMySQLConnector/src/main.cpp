// mysqlx是基于X协议的
#include "xdevapi.h"

int main()
{
    mysqlx::Session sess("localhost", 33060, "root", "root", "personalServerDB");
    
    return 0;
}