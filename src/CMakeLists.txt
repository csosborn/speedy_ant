
add_library(mylib
    mylib/mylib.cpp
)
target_include_directories(mylib PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(myapp
    myapp/main.cpp
)

target_link_libraries(myapp mylib)

add_executable(speedy_ant
    speedy_ant/main.cpp
)
