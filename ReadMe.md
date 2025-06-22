Personal Server 💽
===========

# Introduction

这是我计划用来运行我的个人网站的服务器项目（目前是半成品，将看心情持续更新 --JcGam1ng



> 本项目使用语言：`C++`
> 开发环境配置：`CMake`+`G++`
> 使用第三方开源库 [cpp-httplib](https://github.com/yhirose/cpp-httplib) 实现HTTP服务



**目录结构：**
```
 .
 ├── build
 ├── dist
 │   ├── PersonalServer
 │   └── TestServer
 ├── docs
 ├── extern
 │   └── cpp-httplib
 ├── modules
 └── scripts
```

- `build`: cmake目录，在此目录执行`cmake ..`进行cmake配置(我用`.gitignore`隐藏掉了)
- `dist`：运行程序以及依赖环境输出目录，可以将模块从此直接复制用于打包部署
 - `PersonalServer`：主程序目录
 - `TestServer`：测试程序目录
- `docs`：文档目录（目前没东西，说实话没想好需要写什么
- `extern`：依赖目录
 - `cpp-httplib`：cpp-httplib库，用于实现http服务
- `modules`：不同程序的目录
- `scripts`：脚本目录，用于存储一些快捷远程部署之类的脚本（因为我在虚拟机上开发，远程部署到服务器上

## Plan
- [ ] 数据库页面（用于用于展示个人作品评价）目前计划使用 `doker`部署`MySQL` + `MySQL Connector` + `nlohmann/json` 实现
    - [ ] 数据库写入（需要验证一个密码，保证外人不可破坏我的数据库，但是其实还有别的被攻击的风险对吧？不过大概率没人攻击我
- [ ] 评论功能
- [ ] 界面美化