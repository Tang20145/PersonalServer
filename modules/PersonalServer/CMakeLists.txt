project(PersonalServer)

file(GLOB SOURCE_FILES src/*.cpp)
# message("SOUCE_FILES :${SOURCE_FILES}")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} pthread)

# 可执行文件目录位置设定
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist/${PROJECT_NAME}/)