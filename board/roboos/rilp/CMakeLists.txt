cmake_minimum_required(VERSION 3.10)
project(rilp VERSION 1.0.0)

set(CMAKE_C_STANDARD 11)

# RILP library
add_library(rilp STATIC
    src/rilp.c
    src/rilp_socket.c
)

# Install headers
target_include_directories(rilp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Test program
add_executable(rilp_test src/rilp_test.c)
target_link_libraries(rilp_test rilp pthread)

# Installation
install(TARGETS rilp ARCHIVE DESTINATION lib)
install(FILES include/rilp.h DESTINATION include)
install(TARGETS rilp_test DESTINATION bin)
