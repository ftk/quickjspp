cmake_minimum_required(VERSION 3.13)
project(quickjspp LANGUAGES CXX)

#set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

add_subdirectory(quickjs)

add_library(quickjspp INTERFACE)
target_link_libraries(quickjspp INTERFACE quickjs)
target_compile_features(quickjspp INTERFACE cxx_std_20)
target_include_directories(quickjspp INTERFACE .)
set_target_properties(quickjspp PROPERTIES PUBLIC_HEADER quickjspp.hpp)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(quickjspp INTERFACE -Wall -Wno-unused-parameter) # enable warnings
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
        target_link_libraries(quickjspp INTERFACE stdc++fs) # required for gcc-8
    endif()
elseif(MSVC)
    add_compile_options(/EHsc) # enable exceptions
endif()

add_executable(qjs qjs.cpp)
target_link_libraries(qjs quickjspp)

# cmake -DBUILD_TESTING=OFF to disable tests
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(test)
endif()

install(TARGETS quickjspp qjs)
