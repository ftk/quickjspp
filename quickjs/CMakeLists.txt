project(quickjs LANGUAGES C)


set(quickjs_src quickjs.c libbf.c libunicode.c libregexp.c cutils.c quickjs-libc.c)


add_library(quickjs ${quickjs_src})
target_compile_definitions(quickjs PRIVATE CONFIG_BIGNUM)
target_compile_definitions(quickjs PUBLIC JS_STRICT_NAN_BOXING)

add_library(quickjs-dumpleaks ${quickjs_src})
target_compile_definitions(quickjs-dumpleaks PRIVATE CONFIG_BIGNUM DUMP_LEAKS=1)
target_compile_definitions(quickjs-dumpleaks PUBLIC JS_STRICT_NAN_BOXING)

if(UNIX)
  find_package(Threads)
  target_link_libraries(quickjs ${CMAKE_DL_LIBS} m Threads::Threads)
  target_link_libraries(quickjs-dumpleaks ${CMAKE_DL_LIBS} m Threads::Threads)
endif()

