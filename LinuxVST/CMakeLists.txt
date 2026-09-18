cmake_minimum_required(VERSION 3.9)
project(sympathetic_strings)

set(CMAKE_CXX_STANDARD 14)
add_compile_options(-O2 -D__cdecl=)

include(Helpers.cmake)

add_subdirectory(include/vstsdk)
add_airwindows_plugin(Sympathetic)
