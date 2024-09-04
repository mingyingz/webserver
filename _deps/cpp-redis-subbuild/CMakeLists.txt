# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.16.3)

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(cpp-redis-populate NONE)

include(ExternalProject)
ExternalProject_Add(cpp-redis-populate
                     "UPDATE_DISCONNECTED" "False" "URL" "https://github.com/cpp-redis/cpp_redis/archive/refs/tags/4.3.1.tar.gz" "URL_HASH" "SHA256=3859289d8254685fc775bda73de03dad27df923423b8ceb375b02d036c03b02f" "UPDATE_DISCONNECTED" "1"
                    SOURCE_DIR          "/home/zhonghm/webserver/webserver/_deps/cpp-redis-src"
                    BINARY_DIR          "/home/zhonghm/webserver/webserver/_deps/cpp-redis-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
)
