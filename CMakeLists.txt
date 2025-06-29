# 校验构建目录是否在项目内
if(NOT "${CMAKE_BINARY_DIR}" MATCHES "${CMAKE_SOURCE_DIR}/build")
  message(FATAL_ERROR "禁止在项目外部构建！请在项目内创建build目录后进入build执行cmake。")
endif()

# 基本配置 =========================================
cmake_minimum_required(VERSION 3.10)

project(PersonalServer VERSION 0.0.1)

# 创建dist目录，用于最终打包
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/dist)

# 全局include目录
include_directories(${CMAKE_SOURCE_DIR}/extern/cpp-httplib
                    ${CMAKE_SOURCE_DIR}/extern/usr/include/mysql-cppconn)

# 全局链接库目录
link_directories("${CMAKE_SOURCE_DIR}/extern/usr/lib/x86 64-linux-gnu")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 添加模块 =========================================
add_subdirectory(modules)