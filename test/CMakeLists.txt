include_directories(..)

macro(make_test name src)
add_executable(${name} ${src})
target_link_libraries(${name} quickjs-dumpleaks)
add_test(${name} ${name})
endmacro()

foreach(test
        value class exception example multicontext conversions point
        )
make_test(${test} ${test}.cpp)
make_test(${test}-fastclass ${test}.cpp)
target_compile_definitions(${test}-fastclass PUBLIC QJS_CLASS_INTERFACE=2)
endforeach()

