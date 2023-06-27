cmake_minimum_required(VERSION 3.0.0)
project(RetroSim VERSION 0.1.0 LANGUAGES C CXX)

# include(CTest)
# enable_testing()

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

add_executable(out ${SOURCES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)