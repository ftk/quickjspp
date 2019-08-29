cmake_minimum_required(VERSION 3.8)
project(quickjspp)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

#if(GNU)
#    add_compile_options(-Wall -Wno-unused-parameter)
#    endif()

add_subdirectory(quickjs)
add_executable(quickjspp main.cpp)
target_link_libraries(quickjspp quickjs )
