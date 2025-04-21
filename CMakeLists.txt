cmake_minimum_required(VERSION 3.10)
project(RTS_Project VERSION 1.0.0 LANGUAGES CXX)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 빌드 타입 설정
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 컴파일러 경고 설정
if(MSVC)
    add_compile_options(/W4 /WX /wd4244)
    add_definitions(-D_WIN32_WINNT=0x0A00)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# 프로젝트 하위 디렉토리 추가
add_subdirectory(Core)
add_subdirectory(RTS)

# 출력 디렉토리 설정
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


