cmake_minimum_required(VERSION 3.10)

project(Protobuf VERSION 1.0.0 LANGUAGES CXX)

if(MSVC)
    get_filename_component(ROOT_DIR "${CMAKE_SOURCE_DIR}" DIRECTORY)
    set(SHARED_LIBRARY_DIRS "${ROOT_DIR}/Library")
else()
    find_package(Boost REQUIRED)
endif()

# �ҽ� ���� �ڵ� Ž��
file(GLOB_RECURSE PROTOBUF_SOURCES 
    "*.cpp"
    "*.cc"
    ""
    "*.h"
    "*.hpp"
)

# ���������ϵ� ��� ���� ����
list(FILTER PROTOBUF_SOURCES EXCLUDE REGEX "stdafx\\.cpp$")
list(FILTER PROTOBUF_SOURCES EXCLUDE REGEX "stdafx\\.h$")

# Core ���̺귯�� ����
add_library(Protobuf STATIC
    ${PROTOBUF_SOURCES}
    stdafx.cpp
    stdafx.h
)

# ���������ϵ� ��� ����
target_precompile_headers(Protobuf 
    PRIVATE
        stdafx.h
)

target_compile_definitions(Protobuf PUBLIC PROTOBUF_USE_DLLS GRP_DLL_IMPORTS GRPC_DLL_IMPORTS GRPCXX_DLL_IMPORTS ABSL_CONSUME_DLL)

if(MSVC)
target_include_directories(Protobuf
    PRIVATE
        ${CMAKE_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SHARED_LIBRARY_DIRS}/include
)
target_compile_options(Protobuf PUBLIC /wd4251)
else()

target_include_directories(Protobuf
    PRIVATE
        ${CMAKE_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
)
endif()

# ��� ���丮 ����
set_target_properties(Protobuf PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

