#ifndef SQLAPI_H_
#define SQLAPI_H_

#include "xdevapi.h"
#include "json.hpp"
#include <string>
using json = nlohmann::json;

using namespace std;

// sql使用会话
extern mysqlx::Session g_nSess;

string getWatchListFullViewJsonString();



#endif