#ifndef ACCEL_DATA_STREAMER_H
#define ACCEL_DATA_STREAMER_H

#include "accel_interface.h"
#include "accel_data_queue.h"
#include "tcp_interface.h"

void timer_isr(void);
void data_processing_task(TcpConfig tcp_config);
uint32_t calculate_accel_magnitude(int16_t a_x_counts, int16_t a_y_counts, int16_t a_z_counts);
uint64_t isqrt64(uint64_t n);



#endif