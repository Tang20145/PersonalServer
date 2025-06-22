Personal Server 💽
===========


这是我计划用来运行我的个人网站的服务器项目（目前是半成品，将看心情持续更新 --JcGam1ng



> 本项目使用语言：`C++`
> 开发环境配置：`CMake`+`G++`
> 使用第三方开源库 [cpp-httplib](https://github.com/yhirose/cpp-httplib) 实现HTTP服务



**目录结构：**

> .
> ├── build					cmake目录，在此目录执行`cmake ..`进行cmake配置(我用`.gitignore`隐藏掉了)
> ├── dist					 运行程序以及依赖环境输出目录，可以将模块从此直接复制用于打包部署	
> │   ├── PersonalServer	   	主程序目录
> │   └── TestServer		   	测试程序目录
> ├── docs					 文档目录（目前没东西，说实话没想好需要写什么
> ├── extern				   依赖目录
> │   └── cpp-httplib		  	cpp-httplib库，用于
> ├── modules				  不同模块的代码目录
> └── scripts				  脚本目录，用于存储一些快捷远程部署之类的脚本（因为我在虚拟机上开发，远程部署到服务器上