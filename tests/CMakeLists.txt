add_executable(mylib_test
    mylib/mylib_test.cpp
)

target_link_libraries(mylib_test mylib)

# Add testing support
enable_testing()
add_test(NAME mylib_test COMMAND mylib_test)
