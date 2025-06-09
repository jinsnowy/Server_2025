cmake_minimum_required(VERSION 3.10)

project(Protobuf VERSION 1.0.0 LANGUAGES CXX)

if(MSVC)
    get_filename_component(ROOT_DIR "${CMAKE_SOURCE_DIR}" DIRECTORY)
    set(SHARED_LIBRARY_DIRS "${ROOT_DIR}/Library")
else()
    find_package(Boost REQUIRED)
endif()

# 소스 파일 자동 탐색
file(GLOB_RECURSE PROTOBUF_SOURCES 
    "*.cpp"
    "*.cc"
    "*.h"
    "*.hpp"
)

# 프리컴파일된 헤더 파일 제외
list(FILTER PROTOBUF_SOURCES EXCLUDE REGEX "stdafx\\.cpp$")
list(FILTER PROTOBUF_SOURCES EXCLUDE REGEX "stdafx\\.h$")

# Core 라이브러리 설정
add_library(Protobuf STATIC
    ${PROTOBUF_SOURCES}
    stdafx.cpp
    stdafx.h
)

# 프리컴파일된 헤더 설정
target_precompile_headers(Protobuf 
    PRIVATE
        stdafx.h
)

# 컴파일 옵션
target_compile_definitions(Protobuf PRIVATE PROTOBUF_USE_DLLS)

if(MSVC)
target_compile_options(Protobuf PRIVATE /wd4251)
  
target_include_directories(Protobuf
    PRIVATE
        ${CMAKE_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SHARED_LIBRARY_DIRS}/include
)
else()

target_include_directories(Protobuf
    PRIVATE
        ${CMAKE_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
)
endif()

# 출력 디렉토리 설정
set_target_properties(Protobuf PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

