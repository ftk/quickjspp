cmake_minimum_required(VERSION 3.13)
project(quickjspp LANGUAGES CXX)

#set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

if(CMAKE_COMPILER_IS_GNUCC)
    add_compile_options(-Wall -Wno-unused-parameter)
endif()

add_subdirectory(quickjs)

add_library(quickjspp INTERFACE)
target_link_libraries(quickjspp INTERFACE quickjs)
target_compile_features(quickjspp INTERFACE cxx_std_17)
set_target_properties(quickjspp PROPERTIES PUBLIC_HEADER quickjspp.hpp)
target_include_directories(quickjspp INTERFACE .)

add_executable(qjs qjs.cpp)
target_link_libraries(qjs quickjspp)

enable_testing()
add_subdirectory(test)

install(TARGETS quickjspp qjs)
