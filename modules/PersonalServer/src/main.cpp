#include "httplib.h"
#include "sqlApi.h"

using namespace httplib;

int main()
{
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
        res.set_content(sqlApi::getWatchListFullViewJsonString(),"application/json");
        });

    svr.listen("0.0.0.0",80);
    
}