#include "i2c_mock.h"

// Global Variables for Accel Statefulness
SAMPLING_STATE sampling_enabled = SAMPLING_DISABLED;
HPF_STATE hpf_enabled = HPF_DISABLED;
uint8_t test_counter = 0;
float a_x = 0;
float a_y = 0;
float a_z = 0;

int i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t *buf, size_t len)
{
    int ret_code = -1;
    if(addr != I2C_ADDR_ACCEL)
    {
        printf("Invalid read address!\n");
        ret_code = -1;
    }
    else 
    {
        ACCEL_REGISTER_MAP curr_register = reg;
        switch(curr_register) 
        {
            // Only adding register read cases needed to test the interface implementation
            case(CTRL):
            {
                buf[0] = 0x00 | sampling_enabled | (hpf_enabled << 1);
                
                ret_code = 0;
                break;
            }
            case(WHO_AM_I):
            {
                buf[0] = WHO_AM_I_VALUE;
                printf("who am I?\n");
                ret_code = 0;
                break;
            }
            case(OUT_X_L):
            {
                AccelMeasurementBytes new_meas;
                convert_float_to_signed_12_bit(a_x,&new_meas.OUT_X_H,&new_meas.OUT_X_L);
                convert_float_to_signed_12_bit(a_y,&new_meas.OUT_Y_H,&new_meas.OUT_Y_L);
                convert_float_to_signed_12_bit(a_z,&new_meas.OUT_Z_H,&new_meas.OUT_Z_L);
                
                memcpy(buf,&new_meas,len);
                ret_code = 0;
                break;
            }
            default:
                printf("Invalid Read Request!\n");
                ret_code = -2;
        }
    }
    return ret_code;
}

int i2c_write_reg(uint8_t addr, uint8_t reg, const uint8_t *buf, size_t len)
{
    int ret_code = -1;
    if(addr != I2C_ADDR_ACCEL)
    {
        ret_code = -1;
    }
    if(len != 1)
    {
        ret_code = -3;
        printf("invalid length to write!\n");
    }
    else 
    {
        ACCEL_REGISTER_MAP curr_register = reg;
        switch(curr_register) 
        {
            case(CTRL):
            {
                sampling_enabled = buf[0] & 0x01;
                hpf_enabled = (buf[0] & 0x02) >> 1;
                ret_code = 0; 
                printf("Sampling enabled: %d, hpf enabled %d \n", sampling_enabled, hpf_enabled);
                break; 
            }
            default:
                printf("Invalid Write Request!\n");
                ret_code = -2;
        }
    }
    return ret_code;
}
void i2c_mock_step(void)
{
    a_x = 2.19;
    a_y = 3.72;
    a_z = 1.21;

    test_counter++;
}

void convert_float_to_signed_12_bit(float input_value, uint8_t* msb, uint8_t* lsb) 
{
    int16_t coersed_value = (int16_t) (input_value * 2048 / MAX_SUPPORTED_Gs);
    if(coersed_value < -2048)
    {
        coersed_value = -2048;
    }
    else if (coersed_value > 2047)
    {
        coersed_value = 2047;
    }

    coersed_value = coersed_value << 4;
    msb[0] = (coersed_value >> 8) & 0xFF;
    lsb[0] = coersed_value & 0xFF;
}
