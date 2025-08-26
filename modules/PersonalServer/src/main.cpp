#include "httplib.h"
#include "sqlApi.h"

using namespace httplib;

// 收到的请求
void print(const httplib::Request & l_rRequst);

int main()
{
    //sqlApi::init();


    Server svr;

    // 页面
    // 主页
    svr.Get("/", [](const httplib::Request &, httplib::Response &res)
            { res.set_file_content("view/profile.html"); });
    // css 文件
    svr.set_mount_point("/", "view");

    

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