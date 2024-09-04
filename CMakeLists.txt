cmake_minimum_required(VERSION 3.10)

project(Server)

set(CMAKE_CXX_STANDARD 14)

# Set some options internally used in Photon
set(PHOTON_ENABLE_URING OFF CACHE INTERNAL "Enable iouring")
set(PHOTON_CXX_STANDARD 14 CACHE INTERNAL "C++ standard")

# 1. Fetch Photon repo with specific tag or branch
include(FetchContent)
FetchContent_Declare(
        photon
        GIT_REPOSITORY https://github.com/alibaba/PhotonLibOS.git
        GIT_TAG main
)
FetchContent_MakeAvailable(photon)



add_executable(server main.cpp
               ./threadpool/threadpool.h
               ./http/http_conn.cpp
               ./http/http_conn.h
               ./lock/locker.h
               ./log/log.cpp
               ./log/log.h
               ./log/block_queue.h
               ./CGImysql/sql_connection_pool.cpp
               ./CGImysql/sql_connection_pool.h)

find_package(Threads REQUIRED)
find_library(MYSQLCLIENT_LIB mysqlclient)
target_link_libraries(server PRIVATE ${CMAKE_THREAD_LIBS_INIT} ${MYSQLCLIENT_LIB} photon_static)
