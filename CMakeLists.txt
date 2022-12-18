cmake_minimum_required(VERSION 3.15)
project(broadcast-net C)
find_package(OpenSSL)
include_directories(include/)

add_library(bcnet SHARED
    src/bcnet/auth.c
    src/bcnet/bcnet.c
)
if (WIN32)
    target_link_libraries(bcnet
        Ws2_32.lib
    )
endif()

add_executable(test EXCLUDE_FROM_ALL
    src/test/main.c
)
target_link_libraries(test
    bcnet
    OpenSSL
)