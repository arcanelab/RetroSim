cmake_minimum_required(VERSION 3.0.0)
project(RetroSim VERSION 0.1.0 LANGUAGES C CXX)

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
# include(CTest)
# enable_testing()

if(TARGET SDL2::SDL2main)
    # It has an implicit dependency on SDL2 functions, so it MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
    add_executable(RetroSim "")
    target_link_libraries(RetroSim PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(RetroSim PRIVATE SDL2::SDL2)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

target_sources(RetroSim PRIVATE ${SOURCES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
