# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.16.3)

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(rapidxml-populate NONE)

include(ExternalProject)
ExternalProject_Add(rapidxml-populate
                     "UPDATE_DISCONNECTED" "False" "URL" "https://sourceforge.net/projects/rapidxml/files/rapidxml/rapidxml%201.13/rapidxml-1.13.zip/download" "URL_HASH" "SHA256=c3f0b886374981bb20fabcf323d755db4be6dba42064599481da64a85f5b3571" "UPDATE_DISCONNECTED" "1"
                    SOURCE_DIR          "/home/zhonghm/webserver/webserver/_deps/rapidxml-src"
                    BINARY_DIR          "/home/zhonghm/webserver/webserver/_deps/rapidxml-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
)
