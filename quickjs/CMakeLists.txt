project(quickjs LANGUAGES C)

if(MSVC)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(arch 64)
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(arch 32)
  endif()

  # compiling quikjs with msvc is not supported
  # download quickjs that was cross-compiled with mingw
  # https://nightly.link/ftk/quickjspp/workflows/quickjsdll/master/dlls%20-flto%20-O3.zip
  if(NOT EXISTS ${CMAKE_BINARY_DIR}/libquickjs${arch}.dll)
    file(DOWNLOAD
            "https://nightly.link/ftk/quickjspp/workflows/quickjsdll/master/dlls%20-flto%20-O3.zip"
            libquickjs.zip
            TLS_VERIFY ON SHOW_PROGRESS)
    file(ARCHIVE_EXTRACT INPUT libquickjs.zip DESTINATION ${CMAKE_BINARY_DIR})
  endif()
  add_library(quickjs SHARED IMPORTED GLOBAL)
  add_library(quickjs-dumpleaks ALIAS quickjs)
  set_target_properties(quickjs PROPERTIES
          IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/libquickjs${arch}.dll
          IMPORTED_IMPLIB ${CMAKE_BINARY_DIR}/libquickjs${arch}.a
          )
  include(GNUInstallDirs)
  install(FILES quickjs.h quickjs-libc.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/quickjs)
  install(FILES ${CMAKE_BINARY_DIR}/libquickjs32.dll ${CMAKE_BINARY_DIR}/libquickjs64.dll
          DESTINATION ${CMAKE_INSTALL_BINDIR})
  install(FILES ${CMAKE_BINARY_DIR}/libquickjs32.a ${CMAKE_BINARY_DIR}/libquickjs64.a
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/quickjs)
else(MSVC)
  # compile quickjs
  file(STRINGS VERSION version)
  set(quickjs_src quickjs.c libbf.c libunicode.c libregexp.c cutils.c quickjs-libc.c)
  set(quickjs_def CONFIG_VERSION="${version}" _GNU_SOURCE CONFIG_BIGNUM)

  add_library(quickjs ${quickjs_src})
  target_compile_definitions(quickjs PRIVATE ${quickjs_def})
  set_target_properties(quickjs PROPERTIES PUBLIC_HEADER "quickjs.h;quickjs-libc.h")

  add_library(quickjs-dumpleaks ${quickjs_src})
  target_compile_definitions(quickjs-dumpleaks PRIVATE ${quickjs_def} DUMP_LEAKS=1)

  if(UNIX OR MINGW)
    find_package(Threads)
    target_link_libraries(quickjs ${CMAKE_DL_LIBS} m Threads::Threads)
    target_link_libraries(quickjs-dumpleaks ${CMAKE_DL_LIBS} m Threads::Threads)
  endif()

  # install
  include(GNUInstallDirs)
  install(TARGETS quickjs
          PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/quickjs
          ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}/quickjs
          LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}/quickjs
          )
endif(MSVC)
