cmake_minimum_required(VERSION 3.8)

project(leyval LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic-errors")
set (CMAKE_CXX_STANDARD 20)
set(CMAKE_LINKER_TYPE "MOLD")

add_compile_definitions("DEBUG=$<CONFIG:Debug>") # https://stackoverflow.com/a/72330784

file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.hpp)
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# spdlog ####################
find_package(spdlog REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)
#############################
