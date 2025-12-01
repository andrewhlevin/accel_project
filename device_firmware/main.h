

#ifndef MAIN_H
#define MAIN_H
#ifdef __cplusplus
extern "C" {
#endif
#include "accel_interface.h"
#include "accel_data_queue.h"
#include "tcp_interface.h"
#include <time.h>

#define OUTPUT_MESSAGE_SIZE sizeof(AccelDataPayload) + sizeof(TcpHeader) + sizeof(uint16_t)

typedef struct __attribute__((packed)){
    uint64_t timestamp_ms;
    int16_t a_x_mg;
    int16_t a_y_mg;
    int16_t a_z_mg;
    uint32_t a_magnitude_counts;
} AccelDataPayload;

void timer_isr(void);
void data_processing_task(TcpConfig tcp_config);
uint64_t get_time_in_ms();

AccelDataPayload construct_accel_data_payload(AccelMeasurement meas);
uint32_t calculate_accel_magnitude(int16_t a_x_counts, int16_t a_y_counts, int16_t a_z_counts);
uint64_t isqrt64(uint64_t n);
#ifdef __cplusplus
}
#endif

#endif
