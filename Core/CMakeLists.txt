cmake_minimum_required(VERSION 3.10)

find_package(Boost REQUIRED)

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
add_library(core STATIC
    ${CORE_SOURCES}
    stdafx.cpp
    stdafx.h
)
# 프리컴파일된 헤더 설정
target_precompile_headers(core 
    PRIVATE
        stdafx.h
)

# 헤더 파일 경로 설정
target_include_directories(core
    PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
)

message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")

target_link_libraries(core PRIVATE Boost::boost)

# 컴파일러 경고 설정
if(MSVC)
    target_compile_options(core PRIVATE /W4 /WX /wd4244)
    add_definitions(-D_WIN32_WINNT=0x0A00)
else()
    target_compile_options(core PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# 출력 디렉토리 설정
set_target_properties(core PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

