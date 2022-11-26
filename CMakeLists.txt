cmake_minimum_required(VERSION 3.20)
project(broadcast-net C)
find_package(OpenSSL)
include_directories(include/)

add_library(bcnet SHARED
    src/bcnet/auth.c
)

if(WIN32)
    target_sources(bcnet PRIVATE
        src/bcnet/win32/bcnet.c
    )
else()
    target_sources(bcnet PRIVATE
        src/bcnet/unix/bcnet.c
    )
endif()

add_executable(test EXCLUDE_FROM_ALL
    src/test/main.c
)
target_link_libraries(test
    bcnet
)