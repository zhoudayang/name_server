cmake_minimum_required(VERSION 3.9)
project(name_server)
add_compile_options(-std=c++11 -Wall -Werror)

find_package(Boost REQUIRED regex)

link_libraries(muduo_base_cpp11 muduo_net_cpp11 hiredis ${Boost_LIBRARIES} pthread)

add_executable(name_server
        name.cc
        name_server.cc
        command.cc
        redis_cli.cc
        main.cc
        )