#include "spdlog/spdlog.h"

// 封装日志
#ifndef JCLOG_H_
#define JCLOG_H_

#define JC_DEBUG(...) \
    spdlog::debug()

#endif