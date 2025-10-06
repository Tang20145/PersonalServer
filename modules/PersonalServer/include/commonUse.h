#include <vector>
#include <string>

#ifndef _COMMON_USE_H_
#define _COMMON_USE_H_

namespace commonUse
{
    // 将以指定字符分隔的字符串解析成一个vector
    std::vector<std::string> vecStrGetArraySplitBy(const char* p_sFrom,char l_cSplit);
}

#endif