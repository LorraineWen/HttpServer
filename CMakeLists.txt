cmake_minimum_required(VERSION 3.10)
project(httpserver)


#设置编译文件生成路径
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build)

#添加模块
add_subdirectory(src)