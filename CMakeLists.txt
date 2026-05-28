cmake_minimum_required(VERSION 3.14)
project(TraductorDefinitivo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(CURL REQUIRED)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(TraductorDefinitivo ${SOURCES})
target_link_libraries(TraductorDefinitivo PRIVATE CURL::libcurl)
