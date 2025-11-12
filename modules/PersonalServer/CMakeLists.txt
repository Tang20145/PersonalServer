project(PersonalServer)

file(GLOB SOURCE_FILES src/*.cpp)
# message("SOUCE_FILES :${SOURCE_FILES}")

#头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_compile_definitions(${PROJECT_NAME}
    PRIVATE SPDLOG_DEBUG_ON
)

target_link_libraries(${PROJECT_NAME} pthread mysqlcppconnx)

# 可执行文件目录位置设定
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist/${PROJECT_NAME}/)