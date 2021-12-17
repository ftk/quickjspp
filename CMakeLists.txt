cmake_minimum_required(VERSION 3.18)
project(quickjspp LANGUAGES CXX)

#set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

add_subdirectory(quickjs)

add_library(quickjspp INTERFACE)
target_link_libraries(quickjspp INTERFACE quickjs)
target_compile_features(quickjspp INTERFACE cxx_std_20)
target_include_directories(quickjspp INTERFACE .)
set_target_properties(quickjspp PROPERTIES PUBLIC_HEADER quickjspp.hpp)

if(CMAKE_COMPILER_IS_GNUCC)
    add_compile_options(-Wall -Wno-unused-parameter)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
        target_link_libraries(quickjspp INTERFACE stdc++fs) # required for gcc-8
    endif()
endif()
if(MSVC)
    add_compile_options(/EHsc)
endif()

add_executable(qjs qjs.cpp)
target_link_libraries(qjs quickjspp)

# cmake -DBUILD_TESTING=OFF to disable tests
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(test)
endif()

install(TARGETS quickjspp qjs)
