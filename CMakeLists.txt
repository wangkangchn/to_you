cmake_minimum_required(VERSION 3.0.0)

project(to_you)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -Wall -fPIC")


if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE DEBUG)
endif()

set(WKANGK_TOOLS_DIR "/home/wkangk/tools")
include_directories("${WKANGK_TOOLS_DIR}")
include_directories("${PROJECT_SOURCE_DIR}")

# (1) 单元测试
enable_testing()
add_subdirectory(test)

