cmake_minimum_required(VERSION 3.10)

project(rangepp_app)

set(BUILD_APP OFF CACHE BOOL "Build the application test")

add_executable(rangepp_app "${CMAKE_CURRENT_SOURCE_DIR}/src.cpp")
target_link_libraries(rangepp_app PRIVATE rangepp)
set_target_properties(rangepp_app PROPERTIES 
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
if (MSVC)
    target_compile_options(rangepp_app PRIVATE "/experimental:module")
endif()