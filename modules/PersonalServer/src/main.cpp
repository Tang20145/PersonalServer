#include "httplib.h"
#include "sqlApi.h"

using namespace httplib;

// 收到的请求
void print(const httplib::Request & l_rRequst);

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
    svr.Get("/WatchListTest", [](const httplib::Request &, httplib::Response &res)
            { res.set_file_content("view/watchListTest.html"); });
    

    // css 文件
    svr.set_mount_point("/css", "./css");

    // 动态资源
    // 观影列表
    svr.Get("/api/WatchListFullView", [](const httplib::Request &l_qRequest, httplib::Response &res){
        printf("get /api/WatchListFullView\n");
        // 准备查找参数
        std::unordered_map<std::string,std::string> l_mRequest;
        l_mRequest["draw"] = (l_qRequest.params.find("draw")!=l_qRequest.params.end()) ? l_qRequest.params.find("draw")->second : "1";
        l_mRequest["page"] = (l_qRequest.params.find("page")!=l_qRequest.params.end()) ? l_qRequest.params.find("page")->second : "1";
        l_mRequest["pageSize"] = (l_qRequest.params.find("pageSize")!=l_qRequest.params.end()) ? l_qRequest.params.find("pageSize")->second : "10";
        l_mRequest["keyword"] = (l_qRequest.params.find("keyword")!=l_qRequest.params.end()) ? l_qRequest.params.find("keyword")->second : "";

        l_mRequest["sortField"] = "id";// 暂时先这样设置

        l_mRequest["countSql"] = "select count(id) from WatchListFullView";// 用于计总数

        l_mRequest["sortDirection"] = (l_qRequest.params.find("sortDirection")!=l_qRequest.params.end()) ? l_qRequest.params.find("sortDirection")->second : "asc";

        l_mRequest["sql"]="SELECT id,name,eng_name,tags AS tags_tags,type,rate,status,DATE_FORMAT(start_time, \"%Y-%m-%d\") AS start_time,DATE_FORMAT(finish_time, \"%Y-%m-%d\") AS finish_time,comment,link FROM WatchListFullView";

        res.set_content(sqlApi::getSqlQueryJsonString(l_mRequest),"application/json");
    });

    // 观影列表
    svr.Get("/api/WatchListFullViewTest", [](const httplib::Request &l_qRequest, httplib::Response &res){
        printf("get /api/WatchListFullView\n");
        // 准备查找参数
        std::unordered_map<std::string,std::string> l_mRequest;
        l_mRequest["draw"] = (l_qRequest.params.find("draw")!=l_qRequest.params.end()) ? l_qRequest.params.find("draw")->second : "1";
        l_mRequest["page"] = (l_qRequest.params.find("start")!=l_qRequest.params.end()) ? l_qRequest.params.find("start")->second : "1";
        l_mRequest["pageSize"] = (l_qRequest.params.find("length")!=l_qRequest.params.end()) ? l_qRequest.params.find("length")->second : "10";
        l_mRequest["keyword"] = (l_qRequest.params.find("keyword")!=l_qRequest.params.end()) ? l_qRequest.params.find("keyword")->second : "";

        l_mRequest["sortField"] = "id";// 暂时先这样设置

        l_mRequest["countSql"] = "select count(id) from WatchListFullView";// 用于计总数

        l_mRequest["sortDirection"] = (l_qRequest.params.find("sortDirection")!=l_qRequest.params.end()) ? l_qRequest.params.find("sortDirection")->second : "asc";

        l_mRequest["sql"]="SELECT id,name,eng_name,tags AS tags_tags,type,rate,status,DATE_FORMAT(start_time, \"%Y-%m-%d\") AS start_time,DATE_FORMAT(finish_time, \"%Y-%m-%d\") AS finish_time,comment,link FROM WatchListFullView";

        res.set_content(sqlApi::getSqlQueryJsonString(l_mRequest),"application/json");
    });

    // 绑定 ​​1024 以下的端口​​需要 root权限，故需要sudo运行
    if (!svr.bind_to_port("0.0.0.0", 80))
    {
        printf("bind_to_port failed\n");
    }
    // 启用端口重用
    SocketOptions opts = [](socket_t sock) {int opt = 1;setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); };
    svr.set_socket_options(opts);
    // 监听
    if (!svr.listen_after_bind())
    {
        printf("listen_after_bind failed\n");
    }
}

void print(const httplib::Request & l_rRequst)
{
    //遍历头
    {
        httplib::Headers l_headers = l_rRequst.headers;
        auto it = l_headers.begin();
        printf("");
        for(;it != l_headers.end();it++)
        {
            printf("%s : %s",(it->first).c_str(),it->second.c_str());
        }
    }

}