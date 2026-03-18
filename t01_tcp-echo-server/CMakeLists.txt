
cmake_minimum_required(VERSION 3.10)

project(TCPEchoServer C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(app
    ../deps/SystemCore/Processing.cpp
    ../deps/SystemCore/Log.cpp
    ../deps/SystemCore/TcpListening.cpp
    ../deps/SystemCore/TcpTransfering.cpp
    ../deps/SystemCore/SystemCommanding.cpp
    ../deps/SystemCore/SystemDebugging.cpp
    main.cpp
    Supervising.cpp
    UserInteracting.cpp
)

target_include_directories(app PRIVATE
    ../deps/SystemCore
)

target_compile_options(app PRIVATE
    -std=gnu++11
    -DCONFIG_PROC_HAVE_LOG=1
    -DCONFIG_PROC_HAVE_CORE_LOG=1
)

find_package(Threads REQUIRED)
target_link_libraries(app PRIVATE Threads::Threads)

