cmake_minimum_required(VERSION 3.14...3.31)

project(mcpelauncherzoom)

set(CMAKE_CXX_STANDARD 23)

set(CPM_SOURCE_CACHE ${PROJECT_SOURCE_DIR}/cpm-cache CACHE PATH "")
include(cmake/CPM.cmake)

add_library(mcpelauncherzoom SHARED
        src/gamewindow.hpp
        src/main.cpp
        src/menu.hpp
        src/zoom.cpp
        src/zoom.hpp)

target_compile_options(mcpelauncherzoom PRIVATE -Wall -Wextra -Wconversion -Wpedantic)

CPMAddPackage("gh:BasedInc/libhat@0.4.0")
CPMAddPackage("gh:stephenberry/glaze@5.0.0")
target_link_libraries(mcpelauncherzoom PRIVATE libhat::libhat glaze::glaze)

if(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
    CPMAddPackage("gh:zyantific/zydis@4.1.1")
    CPMAddPackage("gh:cursey/safetyhook@0.6.5")
    target_link_libraries(mcpelauncherzoom PRIVATE safetyhook::safetyhook)
endif()

install(TARGETS mcpelauncherzoom LIBRARY DESTINATION mods)
