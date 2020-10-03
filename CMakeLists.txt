cmake_minimum_required(VERSION 3.15)
project(quickjspp)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:DebugDLL>")

  if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug") )
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
  endif()
elseif (CMAKE_COMPILER_IS_GNUCC)
  add_compile_options(-Wall -Wno-unused-parameter)
endif()

if(UNIX)
  find_package(Threads)
  set(quickjs_extra_libs ${CMAKE_DL_LIBS} m Threads::Threads)
elseif (WIN32 AND NOT(MSYS OR MINGW))
  # Run vcpkg install pthreads to install PThreads4W support
  find_package(pthreads)
  set(quickjs_extra_libs PThreads4W::PThreads4W)
endif()

file(STRINGS ${PROJECT_SOURCE_DIR}/quickjs/VERSION version)

set(quickjs_dir ${PROJECT_SOURCE_DIR}/quickjs)
set(quickjs_src ${quickjs_dir}/quickjs.c ${quickjs_dir}/libbf.c ${quickjs_dir}/libunicode.c ${quickjs_dir}/libregexp.c ${quickjs_dir}/cutils.c ${quickjs_dir}/quickjs-libc.c)
set(quickjs_def CONFIG_BIGNUM)

if (UNIX)
  set(quickjs_def ${quickjs_def} _GNU_SOURCE)
elseif (WIN32 AND NOT(MSYS OR MINGW))
  add_library(quickjs_win32_compat win32_compat/time.cpp)
  target_include_directories(quickjs_win32_compat INTERFACE ${PROJECT_SOURCE_DIR}/win32_compat)
endif()

if (MSYS OR MINGW)
set(quickjs_def ${quickjs_def} CONFIG_VERSION="\"${version}\"")
else()
set(quickjs_def ${quickjs_def} CONFIG_VERSION="${version}")
endif()

add_library(quickjs ${quickjs_src})
target_compile_definitions(quickjs PRIVATE ${quickjs_def})
target_include_directories(quickjs INTERFACE ${PROJECT_SOURCE_DIR}/quickjs/)
target_link_libraries(quickjs PUBLIC ${quickjs_extra_libs})
if (WIN32 AND NOT(MSYS OR MINGW))
  target_compile_definitions(quickjs PRIVATE 
    ssize_t=SSIZE_T popen=_popen pclose=_pclose
    _CRT_SECURE_NO_WARNINGS
  )
  target_link_libraries(quickjs PRIVATE quickjs_win32_compat)
endif()

add_library(quickjs-dumpleaks INTERFACE)
target_link_libraries(quickjs-dumpleaks INTERFACE quickjs)
target_compile_definitions(quickjs-dumpleaks INTERFACE DUMP_LEAKS=1)

add_library(quickjspp INTERFACE)
target_include_directories(quickjspp INTERFACE ${PROJECT_SOURCE_DIR}/include/)
target_link_libraries(quickjspp INTERFACE quickjs)

add_library(quickjspp-dumpleaks INTERFACE)
target_include_directories(quickjspp-dumpleaks INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(quickjspp-dumpleaks INTERFACE quickjs-dumpleaks)

add_executable(qjs qjs.cpp)
target_link_libraries(qjs quickjspp)

if (BUILD_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()