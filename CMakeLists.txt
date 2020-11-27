cmake_minimum_required(VERSION 3.10)
project(myls)

set(CMAKE_CXX_STANDARD 14)
find_package(Boost REQUIRED COMPONENTS system filesystem program_options)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(myls main.cpp parse_options.cpp comparators.cpp)
target_link_libraries(myls ${Boost_LIBRARIES})