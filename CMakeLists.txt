cmake_minimum_required(VERSION 3.8)

project(OrderBook LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic-errors")

# add_library(mylib Random.hpp)
# set_target_properties(mylib PROPERTIES LINKER_LANGUAGE CXX)

add_executable(${PROJECT_NAME} main.cpp) # add files here

# target_link_libraries(${PROJECT_NAME} mylib)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
