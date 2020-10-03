cmake_minimum_required(VERSION 3.8)
project(quickjspp)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

if(CMAKE_COMPILER_IS_GNUCC)
    add_compile_options(-Wall -Wno-unused-parameter)
endif()

file(STRINGS VERSION quickjs/VERSION)

set(quickjs_src quickjs/quickjs.c quickjs/libbf.c quickjs/libunicode.c quickjs/libregexp.c quickjs/cutils.c quickjs/quickjs-libc.c)
set(quickjs_def CONFIG_VERSION="${version}" CONFIG_BIGNUM)

if (UNIX)
  set(quickjs_def "${quickjs_def}" _GNU_SOURCE)
endif()

add_library(quickjs ${quickjs_src})
target_compile_definitions(quickjs PRIVATE ${quickjs_def} )

add_library(quickjs-dumpleaks ${quickjs_src})
target_compile_definitions(quickjs-dumpleaks PRIVATE ${quickjs_def} DUMP_LEAKS=1)

if(UNIX)
  find_package(Threads)
  set(quickjs_extra_libs ${CMAKE_DL_LIBS} m Threads::Threads)
else if (WIN32)
  find_package(pthreads)
  set(quickjs_extra_libs PThreads4W::PThreads4W)
endif()

target_link_libraries(quickjs ${quickjs_extra_libs})
target_link_libraries(quickjs-dumpleaks ${quickjs_extra_libs})

add_executable(qjs qjs.cpp)
target_link_libraries(qjs quickjs)

enable_testing()
add_subdirectory(test)
