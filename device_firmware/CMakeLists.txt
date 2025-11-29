cmake_minimum_required(VERSION 3.10)

project(AccelStreamerApp C)
add_compile_options(-Wall -Wextra -Wpedantic)
add_executable(AccelStreamerApp
    accel_data_streamer_app.c
    accel_data_streamer_app.h
    i2c_mock.c
    i2c_mock.h
    accel_interface.h
    accel_interface.c
    accel_data_queue.c
    accel_data_queue.h
)