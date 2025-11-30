#include <gtest/gtest.h>
#include "../accel_data_streamer_app.h"   

extern bool bus_error_active;
extern uint8_t sampling_enabled;
extern uint8_t hpf_enabled;

extern float a_x;
extern float a_y;
extern float a_z;

TEST(I2CReadRegTest, ReturnsBusErrorWhenActive)
{
    uint8_t buf[4] = {0};
    bus_error_active = true;

    int ret = i2c_read_reg(I2C_ADDR_ACCEL, WHO_AM_I, buf, 1);

    EXPECT_EQ(ret, BUS_ERROR);
    bus_error_active = false;
}

TEST(I2CReadRegTest, InvalidAddressReturnsError)
{
    uint8_t buf[1] = {0};

    int ret = i2c_read_reg(0xFF, WHO_AM_I, buf, 1);

    EXPECT_EQ(ret, INVALID_ADDRESS);
}

TEST(I2CReadRegTest, WhoAmIRegisterReturnsCorrectValue)
{
    uint8_t buf[1] = {0};

    int ret = i2c_read_reg(I2C_ADDR_ACCEL, WHO_AM_I, buf, 1);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(buf[0], WHO_AM_I_VALUE);
}

TEST(I2CReadRegTest, CtrlRegisterReturnsCorrectBits)
{
    uint8_t buf[1] = {0};

    sampling_enabled = 1;
    hpf_enabled = 0;

    int ret = i2c_read_reg(I2C_ADDR_ACCEL, CTRL, buf, 1);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(buf[0], (uint8_t)(sampling_enabled | (hpf_enabled << 1)));
}

TEST(I2CReadRegTest, OutXLReturnsConvertedMeasurementData)
{
    uint8_t buf[6] = {0};

    a_x = 1.9f;
    a_y = -0.25f;
    a_z = 4.8f;

    int ret = i2c_read_reg(I2C_ADDR_ACCEL, OUT_X_L, buf, sizeof(buf));

    EXPECT_EQ(ret, SUCCESS);

    AccelMeasurementBytes expected_meas;
    convert_float_to_signed_12_bit(a_x, &expected_meas.OUT_X_H, &expected_meas.OUT_X_L);
    convert_float_to_signed_12_bit(a_y, &expected_meas.OUT_Y_H, &expected_meas.OUT_Y_L);
    convert_float_to_signed_12_bit(a_z, &expected_meas.OUT_Z_H, &expected_meas.OUT_Z_L);

    // Compare raw bytes
    EXPECT_EQ(memcmp(buf, &expected_meas, sizeof(expected_meas)), 0);
}

TEST(I2CReadRegTest, InvalidRegisterReturnsError)
{
    uint8_t buf[1] = {0};

    int ret = i2c_read_reg(I2C_ADDR_ACCEL, 0xFF, buf, 1);

    EXPECT_EQ(ret, INVALID_READ_REQUEST);
}

TEST(ConvertSigned12Bit, MaxPositiveValue) {
    // Max Positive is 0x7FF0
    uint8_t msb = 0x7F; 
    uint8_t lsb = 0xF0;
    EXPECT_EQ(convert_signed_12_bit_to_int16(lsb, msb), 2047);
}

TEST(ConvertSigned12Bit, MaxNegativeValue) {
    // Max Negative is 0x8000
    uint8_t msb = 0x80;
    uint8_t lsb = 0x00;
    EXPECT_EQ(convert_signed_12_bit_to_int16(lsb, msb), -2048);
}

TEST(AccelQueue, SimpleEnqueueDequeue) {
    
    AccelDataQueue queue;
    queue_init(&queue);

    AccelMeasurement m = {100, 250, -300};
    EXPECT_EQ(queue_enqueue(&queue, m), 0);

    AccelMeasurement out;
    EXPECT_EQ(queue_dequeue(&queue, &out), 0);
    EXPECT_EQ(out.a_x_counts, 100);
    EXPECT_EQ(out.a_y_counts, 250);
    EXPECT_EQ(out.a_z_counts, -300);

    EXPECT_TRUE(queue_is_empty(&queue));
}

TEST(AccelQueue, CheckForFull) {
    AccelDataQueue queue;
    queue_init(&queue);

    AccelMeasurement m = {0, 0, 0};
    for (int i = 0; i < QUEUE_SIZE; i++) {
        EXPECT_EQ(queue_enqueue(&queue, m), 0);
    }

    EXPECT_EQ(queue_enqueue(&queue, m), -1);
    EXPECT_TRUE(queue_is_full(&queue));
}

TEST(AccelQueue, CheckForEmpty) {
    AccelDataQueue queue;
    queue_init(&queue);

    AccelMeasurement m = {0, 0, 0};
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(queue_enqueue(&queue, m), 0);
    }

    AccelMeasurement out;
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(queue_dequeue(&queue, &out), 0);
    }

    // Queue should now be empty
    EXPECT_EQ(queue_dequeue(&queue, &out), -1);
    EXPECT_TRUE(queue_is_empty(&queue));
}

TEST(TcpInterface, NominalGeneration) {
    uint8_t payload[4] = {0x01, 0x03, 0x05, 0x07};
    uint8_t output[100] = {0};
    int ret = generate_tcp_msg(payload, sizeof(payload), output, sizeof(output));

    EXPECT_EQ(ret, 0);

    // Check header bytes
    EXPECT_EQ(output[0], 0x01);
    EXPECT_EQ(output[1], 0x02);

    // Check sample number (first call = 0)
    uint32_t sample_number;
    memcpy(&sample_number, output + 2, sizeof(uint32_t));
    EXPECT_EQ(sample_number, 0);

    // Check payload copied correctly
    EXPECT_EQ(output[6], 0x01);
    EXPECT_EQ(output[7], 0x03);
    EXPECT_EQ(output[8], 0x05);
    EXPECT_EQ(output[9], 0x07);

    // Check CRC
    uint16_t expected_crc = crc_16(output, 6 + 4); // header + payload
    uint16_t msg_crc;
    memcpy(&msg_crc, output + 6 + 4, sizeof(uint16_t));
    EXPECT_EQ(msg_crc, expected_crc);
}

TEST(TcpInterface, CheckIfBufferNotBigEnough) {
    uint8_t input[6] = {0};
    uint8_t output[5] = {0}; // deliberately too small
    int ret = generate_tcp_msg(input, sizeof(input), output, sizeof(output));
    EXPECT_EQ(ret, -1);
}

TEST(TcpInterface, SampleNumberIncrements) {
    uint8_t payload[1] = {0};
    uint8_t output[20] = {0};

    // First call
    generate_tcp_msg(payload, sizeof(payload), output, sizeof(output));
    uint32_t sample1;
    memcpy(&sample1, output + 2, sizeof(uint32_t));

    // Second call
    generate_tcp_msg(payload, sizeof(payload), output, sizeof(output));
    uint32_t sample2;
    memcpy(&sample2, output + 2, sizeof(uint32_t));

    EXPECT_EQ(sample2, sample1 + 1);
}

TEST(Crc16, TestCase) {
    uint8_t data[5] = {1, 2, 3, 4, 5};
    uint16_t crc = crc_16(data, 5);

    EXPECT_EQ(crc, 0xBB2A); // Looked up the value using online calculator
}