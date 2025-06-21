project(TestServer)

file(GLOB SOURCE_FILES src/*.cpp)
# message("SOUCE_FILES :${SOURCE_FILES}")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})