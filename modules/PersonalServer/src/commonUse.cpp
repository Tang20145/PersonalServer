#include "commonUse.h"
#include <sstream>

namespace commonUse{

    // 将以指定字符分隔的字符串解析成一个vector
    std::vector<std::string> vecStrGetArraySplitBy(const char* p_sFrom,char l_cSplit)
    {
        std::vector<std::string> result;
    
        if (p_sFrom == nullptr) {
            return result;
        }
        
        std::stringstream ss(p_sFrom);
        std::string item;
        
        while (std::getline(ss, item, l_cSplit)) {
            result.push_back(item);
        }
        return result;
    }
}