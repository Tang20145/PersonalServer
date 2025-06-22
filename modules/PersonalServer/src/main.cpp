#include "httplib.h"

using namespace httplib;

int main()
{
    Server svr;

    svr.Get("/", [](const httplib::Request &, httplib::Response &res) {res.set_file_content("view/profile.html");});

    svr.listen("0.0.0.0",80);
    
}