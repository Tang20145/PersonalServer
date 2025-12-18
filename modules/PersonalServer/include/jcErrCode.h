#ifndef JC_ERR_CODE_H_
#define JC_ERR_CODE_H_

// 普通错误码
#define JC_ERR_CODE_OK  0
#define JC_ERR_CODE_ERR -1
// SQL错误码偏移量
#define JC_ERR_CODE_SQL_OFFSET 100
#define JC_ERR_CODE_SQL_ERR_UNKNOWN JC_ERR_CODE_SQL_OFFSET + 1    // 未知错误
#define JC_ERR_CODE_SQL_CONNECTION  JC_ERR_CODE_SQL_OFFSET + 2    //连接错误



#endif