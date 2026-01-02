#include "httplib.h"
#include "sqlApi.h"
#include "jcLog.h"

// 错误码
#include "jcErrCode.h"

using namespace httplib;


int main()
{
    jcLog::vInitLog();
    jcLog::vStartFlushing(jcLog::g_vLoggerManager,std::chrono::seconds(1));//一秒flush

    SPDLOG_LOGGER_INFO(MAIN_LOG,"start init Sql");
    sqlApi::init();
    SPDLOG_LOGGER_INFO(MAIN_LOG,"end init Sql");

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
        } 
    });

    // 管理员登陆接口
    svr.Post("/api/ManagerLogin",[](const httplib::Request &l_Req, httplib::Response &res){
        SPDLOG_LOGGER_INFO(MAIN_LOG,"Recv:{}",l_Req.body.c_str());
        auto json_data = json::parse(l_Req.body);
        std::string input_pwd = json_data["password"];

        // 先把密码和token写死
        if (input_pwd == "testPasswd") {
            std::string token = "testToken"; 
            
            res.status = 200;
            res.set_content("{\"token\": \"" + token + "\"}", "application/json");
        } else {
            res.status = 401; // 未授权
        }

    });

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






