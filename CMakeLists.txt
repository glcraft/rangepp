cmake_minimum_required(VERSION 3.10)

project(rangepp)

set(BUILD_APP OFF CACHE BOOL "Build the application test")

add_library(rangepp INTERFACE)

target_include_directories(rangepp INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
set_target_properties(rangepp PROPERTIES 
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

if (BUILD_APP)
    add_subdirectory(app)
    add_dependencies(rangepp_app rangepp)
endif()