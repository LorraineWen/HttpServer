set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
add_executable(httpserver main.cpp server.cpp httphandler.cpp tools.cpp)

