# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.16.3)

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(rapidyaml-populate NONE)

include(ExternalProject)
ExternalProject_Add(rapidyaml-populate
                     "UPDATE_DISCONNECTED" "False" "URL" "https://github.com/biojppm/rapidyaml/releases/download/v0.5.0/rapidyaml-0.5.0.hpp" "URL_HASH" "SHA256=09a5be64f7add04e24a675704a16b3aea7ed80e9ff73d865a3b92301971d0815" "DOWNLOAD_NO_EXTRACT" "1" "UPDATE_DISCONNECTED" "1"
                    SOURCE_DIR          "/home/zhonghm/webserver/webserver/_deps/rapidyaml-src"
                    BINARY_DIR          "/home/zhonghm/webserver/webserver/_deps/rapidyaml-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
)
