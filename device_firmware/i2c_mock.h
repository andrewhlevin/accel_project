

// Accelerometer emulator for testing purposes
// Simulates a 3-axis accelerometer with configurable parameters
#ifndef I2C_MOCK_H
#define I2C_MOCK_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define I2C_ADDR_ACCEL 0x1A
#define MAX_SUPPORTED_Gs 5
#define WHO_AM_I_VALUE 0x42

typedef enum 
{
    WHO_AM_I = 0x00,
    CTRL = 0x01,
    OUT_X_L = 0x10,
    OUT_X_H = 0x11,
    OUT_Y_L = 0x12,
    OUT_Y_H = 0x13,
    OUT_Z_L = 0x14,
    OUT_Z_H = 0x15,
} ACCEL_REGISTER_MAP;

typedef enum 
{
    SUCCESS = 0,
    INVALID_ADDRESS = -1,
    INVALID_READ_REQUEST = -2,
    BUS_ERROR = -3
} I2C_RET_CODE;


typedef enum
{
    SAMPLING_DISABLED = 0x00,
    SAMPLING_ENABLED = 0x01,
} SAMPLING_STATE;

typedef enum 
{
    HPF_DISABLED = 0x00,
    HPF_ENABLED = 0x01,
} HPF_STATE;

typedef struct 
{
    uint8_t OUT_X_L;
    uint8_t OUT_X_H;
    uint8_t OUT_Y_L;
    uint8_t OUT_Y_H;
    uint8_t OUT_Z_L;
    uint8_t OUT_Z_H;
} AccelMeasurementBytes;

//API Functions
// Returns 0 on success, non-zero on error.
int i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t *buf, size_t len);
int i2c_write_reg(uint8_t addr, uint8_t reg, const uint8_t *buf, size_t len);
void i2c_mock_step(void);
void convert_float_to_signed_12_bit(float input_value, uint8_t* msb,uint8_t* lsb);

#endif // I2C_MOCK_H
