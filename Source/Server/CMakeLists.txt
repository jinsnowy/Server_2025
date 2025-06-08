cmake_minimum_required(VERSION 3.10)

project(Server VERSION 1.0.0 LANGUAGES CXX)

if(MSVC)
    set(SHARED_LIBRARY_DIRS "${CMAKE_SOURCE_DIR}\\..\\Library")
else()
    find_package(Boost REQUIRED)
endif()

# 소스 파일 자동 탐색
file(GLOB_RECURSE SERVER_SOURCES 
    "*.cpp"
    "*.h"
    "*.hpp"
)

# 프리컴파일된 헤더 파일 제외
list(FILTER SERVER_SOURCES EXCLUDE REGEX "stdafx\\.cpp$")
list(FILTER SERVER_SOURCES EXCLUDE REGEX "stdafx\\.h$")


# 메인 실행 파일 설정
add_executable(Server 
${SERVER_SOURCES}
stdafx.h
stdafx.cpp
)

# 컴파일 옵션
target_compile_definitions(Server PRIVATE PROTOBUF_USE_DLLS)

target_precompile_headers(Server 
    PRIVATE
        stdafx.h
)

# 헤더 Include
if(MSVC)
target_include_directories(Server
    PRIVATE
        ${CMAKE_SOURCE_DIR}\\Source
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SHARED_LIBRARY_DIRS}\\include
)
else()
target_include_directories(Server
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
)
endif()

# Core 라이브러리 연결
if(MSVC)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SHARED_LIB_LINK_DIRS ${SHARED_LIBRARY_DIRS}\\lib\\Debug)
else()
    set(SHARED_LIB_LINK_DIRS ${SHARED_LIBRARY_DIRS}\\lib)
endif()
    
message(STATUS "SHARED_LIB_LINK_DIRS: ${SHARED_LIB_LINK_DIRS}")

target_link_directories(Server PRIVATE 
    ${CMAKE_BINARY_DIR}\\lib
    ${SHARED_LIB_LINK_DIRS}
)
    
target_link_libraries(Server PRIVATE 
Core 
Protobuf
boost_system-vc143-mt-gd-x64-1_88
boost_filesystem-vc143-mt-gd-x64-1_88
boost_locale-vc143-mt-gd-x64-1_88
)

else()

endif()

# 출력 디렉토리 설정
set_target_properties(Server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)