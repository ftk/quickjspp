include_directories(..)

if(UNIX AND CMAKE_COMPILER_IS_GNUCC)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
endif()

macro(make_test name)
add_executable(${name} ${name}.cpp)
target_link_libraries(${name} quickjs-dumpleaks)
add_test(${name} ${name})
# try to compile with -DCONFIG_CHECK_JSVALUE
add_library(${name}-checkjsv OBJECT ${name}.cpp)
target_compile_definitions(${name}-checkjsv PRIVATE CONFIG_CHECK_JSVALUE=1)
endmacro()

foreach(test
        value class exception example multicontext
        )
make_test(${test})
endforeach()

