# 校验构建目录是否在项目内
if(NOT "${CMAKE_BINARY_DIR}" MATCHES "${CMAKE_SOURCE_DIR}/build")
  message(FATAL_ERROR "禁止在项目外部构建！请在项目内创建build目录后进入build执行cmake。")
endif()

cmake_minimum_required(VERSION 3.10)

project(PersonalServer VERSION 0.0.1)

# 创建dist目录，用于最终打包
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/dist)
