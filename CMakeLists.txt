cmake_minimum_required(VERSION 2.8.12)
project(sip-server)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(sip-server sip-server.c)
target_link_libraries(sip-server ${CONAN_LIBS})