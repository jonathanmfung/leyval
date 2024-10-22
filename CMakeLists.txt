cmake_minimum_required(VERSION 3.8)

project(leyval LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic-errors")
set (CMAKE_CXX_STANDARD 20)

file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.hpp)
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
