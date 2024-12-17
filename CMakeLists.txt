cmake_minimum_required(VERSION 3.8)

project(leyval LANGUAGES CXX)
set(LIBRARY_NAME ${PROJECT_NAME}lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic-errors")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_LINKER_TYPE "MOLD")
# add_compile_definitions("DEBUG=$<CONFIG:Debug>") # https://stackoverflow.com/a/72330784
# https://discourse.nixos.org/t/get-clangd-to-find-standard-headers-in-nix-shell/11268/10
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

set(HEADERS src/agent.hpp
            src/constants.hpp
            src/exchange.hpp
	    src/fixed_point.hpp
            src/matching_system.hpp
            src/order.hpp
            src/order_book.hpp)

set(SOURCES src/matching_system.cpp
            src/order.cpp
            src/order_book.cpp)

set(UTILS src/my_spdlog.hpp
          src/overloaded.hpp
          src/serializable.hpp)

add_library(${LIBRARY_NAME} SHARED ${SOURCES} ${HEADERS} ${UTILS})
install(TARGETS ${LIBRARY_NAME} )

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBRARY_NAME})

##### 3rd Party Libs ########
find_package(spdlog REQUIRED)
target_link_libraries(${LIBRARY_NAME} PRIVATE spdlog::spdlog)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)

find_package(nlohmann_json REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

install(TARGETS ${PROJECT_NAME})

##### Tests ########
find_package(Catch2 3 REQUIRED)
add_executable(tests test/test_agent.cpp
                     test/test_exchange.cpp
	             test/test_fixed_point.cpp
                     test/test_matching_system.cpp
                     test/test_order.cpp
                     test/test_order_book.cpp)

target_link_libraries(tests PRIVATE ${LIBRARY_NAME}
                            PRIVATE Catch2::Catch2WithMain)
install(TARGETS tests)
