# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/zhonghm/webserver/webserver

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/zhonghm/webserver/webserver

# Include any dependencies generated for this target.
include _deps/photon-build/CMakeFiles/photon_static.dir/depend.make

# Include the progress variables for this target.
include _deps/photon-build/CMakeFiles/photon_static.dir/progress.make

# Include the compile flags for this target's objects.
include _deps/photon-build/CMakeFiles/photon_static.dir/flags.make

# Object files for target photon_static
photon_static_OBJECTS =

# External object files for target photon_static
photon_static_EXTERNAL_OBJECTS = \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/common/alog.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/common/checksum/crc32c.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/common/estring.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/common/event-loop.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/common/executor/executor.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/common/expirecontainer.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/common/identity-pool.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/common/iovector.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/common/memory-stream/memory-stream.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/common/perf_counter.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/common/ring.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/common/stream-messenger/messenger.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/common/stream.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/common/utility.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/common/uuid4.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/ecosystem/redis.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/ecosystem/simple_dom.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/fs/aligned-file.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/fs/async_filesystem.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/fs/exportfs.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/fs/filecopy.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/fs/httpfs/httpfs.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/fs/httpfs/httpfs_v2.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/fs/localfs.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/fs/path.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/fs/subfs.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/fs/throttled-file.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/fs/virtual-file.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/fs/xfile.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/io/reset_handle.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/io/signal.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/net/basic_socket.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/net/curl.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/net/datagram_socket.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/net/http/body.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/net/http/client.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/net/http/cookie_jar.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/net/http/headers.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/net/http/message.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/net/http/server.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/net/http/status.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/net/http/url.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/net/iostream.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/net/kernel_socket.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/net/pooled_socket.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/net/security-context/tls-stream.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/net/utils.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/photon.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/rpc/out-of-order-execution.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/rpc/rpc.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/thread/stack-allocator.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/thread/std-compat.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/thread/thread-key.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/thread/thread-pool.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/thread/thread.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/thread/workerpool.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/io/aio-wrapper.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/io/epoll.cpp.o" \
"/home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_obj.dir/io/epoll-ng.cpp.o"

_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/common/alog.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/common/checksum/crc32c.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/common/estring.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/common/event-loop.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/common/executor/executor.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/common/expirecontainer.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/common/identity-pool.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/common/iovector.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/common/memory-stream/memory-stream.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/common/perf_counter.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/common/ring.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/common/stream-messenger/messenger.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/common/stream.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/common/utility.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/common/uuid4.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/ecosystem/redis.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/ecosystem/simple_dom.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/fs/aligned-file.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/fs/async_filesystem.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/fs/exportfs.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/fs/filecopy.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/fs/httpfs/httpfs.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/fs/httpfs/httpfs_v2.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/fs/localfs.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/fs/path.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/fs/subfs.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/fs/throttled-file.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/fs/virtual-file.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/fs/xfile.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/io/reset_handle.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/io/signal.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/net/basic_socket.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/net/curl.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/net/datagram_socket.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/net/http/body.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/net/http/client.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/net/http/cookie_jar.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/net/http/headers.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/net/http/message.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/net/http/server.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/net/http/status.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/net/http/url.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/net/iostream.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/net/kernel_socket.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/net/pooled_socket.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/net/security-context/tls-stream.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/net/utils.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/photon.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/rpc/out-of-order-execution.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/rpc/rpc.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/thread/stack-allocator.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/thread/std-compat.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/thread/thread-key.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/thread/thread-pool.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/thread/thread.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/thread/workerpool.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/io/aio-wrapper.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/io/epoll.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_obj.dir/io/epoll-ng.cpp.o
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_static.dir/build.make
_deps/photon-build/output/libphoton_sole.a: _deps/photon-build/CMakeFiles/photon_static.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/zhonghm/webserver/webserver/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Linking CXX static library output/libphoton_sole.a"
	cd /home/zhonghm/webserver/webserver/_deps/photon-build && $(CMAKE_COMMAND) -P CMakeFiles/photon_static.dir/cmake_clean_target.cmake
	cd /home/zhonghm/webserver/webserver/_deps/photon-build && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/photon_static.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
_deps/photon-build/CMakeFiles/photon_static.dir/build: _deps/photon-build/output/libphoton_sole.a

.PHONY : _deps/photon-build/CMakeFiles/photon_static.dir/build

_deps/photon-build/CMakeFiles/photon_static.dir/clean:
	cd /home/zhonghm/webserver/webserver/_deps/photon-build && $(CMAKE_COMMAND) -P CMakeFiles/photon_static.dir/cmake_clean.cmake
.PHONY : _deps/photon-build/CMakeFiles/photon_static.dir/clean

_deps/photon-build/CMakeFiles/photon_static.dir/depend:
	cd /home/zhonghm/webserver/webserver && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/zhonghm/webserver/webserver /home/zhonghm/webserver/webserver/_deps/photon-src /home/zhonghm/webserver/webserver /home/zhonghm/webserver/webserver/_deps/photon-build /home/zhonghm/webserver/webserver/_deps/photon-build/CMakeFiles/photon_static.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : _deps/photon-build/CMakeFiles/photon_static.dir/depend

