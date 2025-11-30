#ifndef ACCEL_INTERFACE_H
#define ACCEL_INTERFACE_H

#include "i2c_mock.h"

// Accelerometer interface functions for device firmware
// Provides high-level API for accelerometer operations

// Structure to hold accelerometer measurement data
typedef struct 
{
    int16_t a_x_counts;
    int16_t a_y_counts;
    int16_t a_z_counts;
} AccelMeasurement;

// Initialize the accelerometer
// Returns 0 on success, non-zero on error
int accel_init(HPF_STATE hpf_state);

// Read accelerometer measurements
// Returns 0 on success, non-zero on error
I2C_RET_CODE accel_read_measurement(AccelMeasurement *meas);
int16_t convert_signed_12_bit_to_int16(uint8_t msb, uint8_t lsb);


#endif 