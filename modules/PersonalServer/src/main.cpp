#include "httplib.h"
#include "sqlApi.h"
#include "jcLog.h"

// 错误码
#include "jcErrCode.h"

using namespace httplib;

// 初始化日志
void vInitLog();


int main()
{
    vInitLog();
    jcLog::vStartFlushing(jcLog::g_vLoggerManager,std::chrono::seconds(1));//一秒flush

    SPDLOG_LOGGER_INFO(MAIN_LOG,"start init Sql");
    sqlApi::init();
    SPDLOG_LOGGER_INFO(MAIN_LOG,"end init Sql");

    // // 测试一下查询数据
    // {
    //     std::string l_strOut;
    //     sqlApi::iGetWatchListFullView(l_strOut,2,10);
    // }

    Server svr;

    // 页面
    // 主页
    svr.Get("/", [](const httplib::Request &, httplib::Response &res)
            { res.set_file_content("view/profile.html"); });
    // 其他文件，view目录保存所有的html文件和css文件
    svr.set_mount_point("/", "view");

    // 先实现WatchList
    svr.Get("/WatchList", [](const httplib::Request &l_Req, httplib::Response &res)
            {
        
        std::string l_strPage = l_Req.get_param_value(SQL_PAGE);
        std::string l_strPageSize = l_Req.get_param_value(SQL_PAGE_SIZE);
        SPDLOG_LOGGER_INFO(MAIN_LOG,"Recv:{}",l_Req.target.c_str());

        std::string l_strResponseJson;
        int l_iRet = sqlApi::iGetWatchListFullView(l_strResponseJson,std::stoi(l_strPage),std::stoi(l_strPageSize));
        if(l_iRet==JC_ERR_CODE_OK)
        {
            res.set_content(l_strResponseJson,"application/json");
            res.status = httplib::OK_200;
            SPDLOG_LOGGER_INFO(MAIN_LOG,"iGetWatchListFullView Ok");
        }
        else
        {
            SPDLOG_LOGGER_ERROR(MAIN_LOG,"iGetWatchListFullView Fail:{}",l_iRet);
        } });

    // dataAPI路径标识查询数据库接口，定义/dataAPI的Get路由，Request 会存储用户发送的参数
    svr.Get("/dataAPI", [](const httplib::Request &l_Req, httplib::Response &res)
            {
                // 获取基本参数，页面大小和页码
                std::string l_strTableName = l_Req.get_param_value(SQL_TABLE_NAME);
                std::string l_strPage = l_Req.get_param_value(SQL_PAGE);
                std::string l_strPageSize = l_Req.get_param_value(SQL_PAGE_SIZE);

                int l_iPage = 1;
                int l_iPageSize = SQL_LIMIT_DEFAULT;

                // 页码
                {
                    if (!l_strPage.empty())
                    {
                        try
                        {
                            l_iPage = std::stoi(l_strPage);
                        }
                        catch (const std::exception &e)
                        {
                            printf("[%s]%d :error %s\n", __FUNCTION__, __LINE__, e.what());
                            l_iPage = 1;
                        }
                    }
                    if (l_iPage < 1)
                    {
                        printf("[%s]%d :error page: %d < 1\n", __FUNCTION__, __LINE__, l_iPage);
                        l_iPage = 1;
                    }
                }

                // 偏移
                int l_iOffset = (l_iPage - 1) * l_iPageSize;

                std::string l_strSql = "select name,type,rate from " +
                                       l_strTableName +
                                       " LIMIT " + std::to_string(l_iPageSize) +
                                       " OFFSET " + std::to_string(l_iOffset); });

    // 绑定 ​​1024 以下的端口​​需要 root权限，故需要sudo运行
    if (!svr.bind_to_port("0.0.0.0", 80))
    {
        printf("bind_to_port failed\n");
    }
    // 启用端口重用
    SocketOptions opts = [](socket_t sock)
    {
        int opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    };
    svr.set_socket_options(opts);
    // 监听
    if (!svr.listen_after_bind())
    {
        SPDLOG_LOGGER_ERROR(MAIN_LOG,"listen_after_bind failed");
    }
}

void vInitLog()
{
    // 全局设置日志格式模式
    // 格式解释：
    // [%Y-%m-%d %H:%M:%S.%e] : 日期和时间
    // [%^%l%$]               : 彩色（如果支持）的日志级别
    // [%s:%#]                : 文件名:行号
    // [函数: %!]             : 函数名
    // %v                     : 实际的日志消息
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] [%!] %v");// 设置了这个格式之后，需要用宏来写日志，使用->info函数会匹配不出文件名行号这些

    jcLog::vJcLogInitAsyncLogger(MAIN_LOG,"Main","log/main.log");
    jcLog::vJcLogInitAsyncLogger(SQL_LOG,"SQL","log/sql.log");

    // 确保全局设置也允许 TRACE 级别的日志通过
    spdlog::set_level(spdlog::level::trace);

    MAIN_LOG->set_level(spdlog::level::trace);
    SQL_LOG->set_level(spdlog::level::trace);
}






