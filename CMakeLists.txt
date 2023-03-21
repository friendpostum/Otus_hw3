cmake_minimum_required(VERSION 3.24)
project(hw3)

set(CMAKE_CXX_STANDARD 17)

add_executable(hw3 main.cpp)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_CONTACT example@example.com)
include(CPack)
