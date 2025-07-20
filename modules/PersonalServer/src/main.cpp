#include "httplib.h"
#include "sqlApi.h"

using namespace httplib;

int main()
{
    sqlApi::init();

    // sqlApi::getSqlQueryJsonString("SELECT id,name,eng_name,tags as tags_tags,type,rate,status,DATE_FORMAT(start_time, \"%Y-%m-%d\") AS start_time,DATE_FORMAT(finish_time, \"%Y-%m-%d\") AS finish_time,comment,link FROM WatchListFullView");
    // sqlApi::getWatchListFullViewJsonString();

    Server svr;

    // 页面
    // 主页
    svr.Get("/", [](const httplib::Request &, httplib::Response &res)
            { res.set_file_content("view/profile.html"); });
    svr.Get("/profile.html", [](const httplib::Request &, httplib::Response &res)
            { res.set_file_content("view/profile.html"); });
    // 观影列表
    svr.Get("/WatchList", [](const httplib::Request &, httplib::Response &res)
            { res.set_file_content("view/watchList.html"); });

    // css 文件
    svr.set_mount_point("/css", "./css");

    // 动态资源
    // 观影列表
    svr.Get("/api/WatchListFullView", [](const httplib::Request &l_qRequest, httplib::Response &res){
        // 准备参数
        std::unordered_map<std::string,std::string> l_mRequest;
        l_mRequest["page"] = (l_qRequest.params.find("page")!=l_qRequest.params.end()) ? l_qRequest.params.find("page")->second : "1";
        l_mRequest["pageSize"] = (l_qRequest.params.find("pageSize")!=l_qRequest.params.end()) ? l_qRequest.params.find("pageSize")->second : "10";
        l_mRequest["keyword"] = (l_qRequest.params.find("keyword")!=l_qRequest.params.end()) ? l_qRequest.params.find("keyword")->second : "";

        l_mRequest["sortField"] = "id";// 暂时先这样设置

        l_mRequest["sortDirection"] = (l_qRequest.params.find("sortDirection")!=l_qRequest.params.end()) ? l_qRequest.params.find("sortDirection")->second : "asc";

        res.set_content(sqlApi::getSqlQueryJsonString(l_mRequest),"application/json");
    });

    // 绑定 ​​1024 以下的端口​​需要 root权限，故需要sudo运行
    if (!svr.bind_to_port("0.0.0.0", 80))
    {
        printf("bind_to_port failed\n");
    }
    if (!svr.listen_after_bind())
    {
        printf("listen_after_bind failed\n");
    }
}