#include "httplib.h"
#include "sqlApi.h"

using namespace httplib;

int main()
{
    sqlApi::init();

    // sqlApi::getSqlQueryJsonString("SELECT id,name,eng_name,tags as tags_tags,type,rate,status,DATE_FORMAT(start_time, \"%Y-%m-%d\") AS start_time,DATE_FORMAT(finish_time, \"%Y-%m-%d\") AS finish_time,comment,link FROM WatchListFullView");
    //sqlApi::getWatchListFullViewJsonString();

    Server svr;

    // 页面
    // 主页
    svr.Get("/", [](const httplib::Request &, httplib::Response &res) {
        res.set_file_content("view/profile.html");
        });
    svr.Get("/profile.html", [](const httplib::Request &, httplib::Response &res) {
        res.set_file_content("view/profile.html");
        });
    // 观影列表
    svr.Get("/WatchList", [](const httplib::Request &, httplib::Response &res) {
        res.set_file_content("view/watchList.html");
        });

    // css 文件
    svr.set_mount_point("/css","./css");

    
    // 动态资源
    // 观影列表
    svr.Get("/api/WatchListFullView",[](const httplib::Request &, httplib::Response &res) {
        res.set_content(sqlApi::getSqlQueryJsonString("SELECT id,name,eng_name,tags as tags_tags,type,rate,status,DATE_FORMAT(start_time, \"%Y-%m-%d\") AS start_time,DATE_FORMAT(finish_time, \"%Y-%m-%d\") AS finish_time,comment,link FROM WatchListFullView"),"application/json");
        });

    // 绑定 ​​1024 以下的端口​​需要 root权限，故需要sudo运行
    if(!svr.bind_to_port("0.0.0.0",80))
    {
        printf("bind_to_port failed\n");
    }
    if(!svr.listen_after_bind())
    {
        printf("listen_after_bind failed\n");
    }
    
}