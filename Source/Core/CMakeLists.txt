cmake_minimum_required(VERSION 3.10)

project(Core VERSION 1.0.0 LANGUAGES CXX)

if(MSVC)
    get_filename_component(ROOT_DIR "${CMAKE_SOURCE_DIR}" DIRECTORY)
    set(SHARED_LIBRARY_DIRS "${ROOT_DIR}/Library")
else()
    find_package(Boost REQUIRED)
endif()

# 소스 파일 자동 탐색
file(GLOB_RECURSE CORE_SOURCES 
    "*.cpp"
    "*.h"
    "*.hpp"
)

# 프리컴파일된 헤더 파일 제외
list(FILTER CORE_SOURCES EXCLUDE REGEX "stdafx\\.cpp$")
list(FILTER CORE_SOURCES EXCLUDE REGEX "stdafx\\.h$")

# Core 라이브러리 설정
add_library(Core STATIC
    ${CORE_SOURCES}
    stdafx.cpp
    stdafx.h
)

# 프리컴파일된 헤더 설정
target_precompile_headers(Core 
    PRIVATE
        stdafx.h
)

target_compile_definitions(Core PUBLIC PROTOBUF_USE_DLLS)

if(MSVC)
target_include_directories(Core
    PRIVATE
        ${CMAKE_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SHARED_LIBRARY_DIRS}/include
)
else()
target_include_directories(Core
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
)
endif()

message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "SHARED_LIBRARY_DIRS:" ${SHARED_LIBRARY_DIRS})
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR": ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "CMAKE_BINARY_DIR": ${CMAKE_BINARY_DIR})

# 출력 디렉토리 설정
set_target_properties(Core PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

