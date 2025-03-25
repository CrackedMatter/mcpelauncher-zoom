cmake_minimum_required(VERSION 3.21...3.31)

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

#CPMAddPackage("gh:BasedInc/libhat@0.4.0")
CPMAddPackage("gh:CrackedMatter/libhat#4f19ed72c2a3dc96ccd85cf344428035de53f22b") # hack fix for arm64
CPMAddPackage("gh:stephenberry/glaze@5.0.0")
target_link_libraries(mcpelauncherzoom PRIVATE libhat::libhat glaze::glaze)

if(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
    CPMAddPackage("gh:zyantific/zydis@4.1.1")
    CPMAddPackage("gh:cursey/safetyhook@0.6.5")
    target_link_libraries(mcpelauncherzoom PRIVATE safetyhook::safetyhook)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(mcpelauncherzoom PRIVATE -flto)
    target_link_options(mcpelauncherzoom PRIVATE
            -flto -Wl,--version-script,${PROJECT_SOURCE_DIR}/map.txt -Wl,--no-undefined-version)
    set_target_properties(mcpelauncherzoom PROPERTIES
            LINK_DEPENDS ${PROJECT_SOURCE_DIR}/map.txt)
endif()

install(TARGETS mcpelauncherzoom LIBRARY DESTINATION mods)
