cmake_minimum_required(VERSION 3.10)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

project(AccelStreamerApp C CXX)
add_compile_options(-Wall -Wextra -Wpedantic)
add_executable(AccelStreamerApp
    main.c
    i2c_mock.c
    accel_interface.c
    accel_data_queue.c
    tcp_interface.c
)

add_executable(firmware_tests
    tests/firmware_tests.cpp
    i2c_mock.c
    accel_interface.c
    accel_data_queue.c
    tcp_interface.c
)

target_link_libraries(firmware_tests
        GTest::gtest
        GTest::gtest_main
)

gtest_discover_tests(firmware_tests)

target_link_libraries(AccelStreamerApp m)
