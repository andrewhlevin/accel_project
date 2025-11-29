#include "accel_interface.h"

int accel_init(HPF_STATE hpf_state)
{
    uint8_t read_buf[1] = {0x00};
    
    if(i2c_read_reg(I2C_ADDR_ACCEL, WHO_AM_I, read_buf, 1) != 0)
    {
        return -1;
    }
    else if (read_buf[0] != WHO_AM_I_VALUE)
    {
        return -2;
    }

    uint8_t write_buf[1] = {0x00};
    write_buf[0] = 0x00 | SAMPLING_ENABLED | (hpf_state << 1);

    if(i2c_write_reg(I2C_ADDR_ACCEL, CTRL, write_buf, 1) != 0)
    {
        return -3;
    }
    else 
    {
        return 0;
    }
}

int accel_read_measurement(AccelMeasurement *meas) 
{
    uint8_t read_buf[6];
    if(i2c_read_reg(I2C_ADDR_ACCEL, OUT_X_L, read_buf, 6) != 0)
    {
        return -1;
    }

    meas->a_x_counts = convert_signed_12_bit_to_int16(read_buf[0],read_buf[1]);
    meas->a_y_counts = convert_signed_12_bit_to_int16(read_buf[2],read_buf[3]);
    meas->a_z_counts = convert_signed_12_bit_to_int16(read_buf[4],read_buf[5]);
    return 0;
}

int16_t convert_signed_12_bit_to_int16(uint8_t lsb, uint8_t msb)
{
    int16_t combined_value = (int16_t)((msb << 8) | lsb);

    combined_value = combined_value >> 4;

    return combined_value;
}
