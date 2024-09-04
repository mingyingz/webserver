# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.16.3)

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(rapidjson-populate NONE)

include(ExternalProject)
ExternalProject_Add(rapidjson-populate
                     "UPDATE_DISCONNECTED" "False" "GIT_REPOSITORY" "https://github.com/Tencent/rapidjson.git" "GIT_TAG" "v1.1.0" "PATCH_COMMAND" "git" "apply" "/home/zhonghm/webserver/webserver/_deps/photon-src/ecosystem/patches/rapidjson.patch" "UPDATE_DISCONNECTED" "1"
                    SOURCE_DIR          "/home/zhonghm/webserver/webserver/_deps/rapidjson-src"
                    BINARY_DIR          "/home/zhonghm/webserver/webserver/_deps/rapidjson-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
)
