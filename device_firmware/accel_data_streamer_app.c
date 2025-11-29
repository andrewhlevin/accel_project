#include "accel_data_streamer_app.h"
#include <unistd.h>
static AccelDataQueue data_queue;    


int main() 
{

    int port = 24912;
    int server_fd = 0;
    int socket_fd = 0;

    TcpConfig tcp_config = {server_fd,socket_fd};
    tcp_server_init(&tcp_config, port);
    queue_init(&data_queue);
    int init_code = accel_init(HPF_DISABLED);
    if(init_code != 0)
    {
        printf("Accel Initialization Error! Error Code: %d, Stopping Program\n",init_code);
    }

    while(1)
    {
        timer_isr();
        data_processing_task(tcp_config);
    }

    return 0;
}


uint32_t calculate_accel_magnitude(int16_t a_x_counts, int16_t a_y_counts, int16_t a_z_counts)
{
    uint32_t a_x_counts_squared = (uint32_t) a_x_counts * a_x_counts;
    uint32_t a_y_counts_squared = (uint32_t) a_y_counts * a_y_counts;
    uint32_t a_z_counts_squared = (uint32_t) a_z_counts * a_z_counts;

    return isqrt64(a_x_counts_squared + a_y_counts_squared + a_z_counts_squared);
}

// Bitwise integer square root for 64-bit unsigned integers
uint64_t isqrt64(uint64_t n) {
    uint64_t res = 0;
    uint64_t bit = 1ULL << 62;  // The second-to-top bit for 64-bit

    // "bit" starts at the highest power of four <= n
    while (bit > n)
        bit >>= 2;

    while (bit != 0) {
        if (n >= res + bit) {
            n -= res + bit;
            res = (res >> 1) + bit;
        } else {
            res >>= 1;
        }
        bit >>= 2;
    }

    return res;
}


void timer_isr() 
{    
    static int down_sample_count = 0;
    if(down_sample_count == 3)
    {
        i2c_mock_step();
        down_sample_count = 0;
    }
    else 
    {
        down_sample_count++;
    }

    AccelMeasurement new_meas;
    int meas_code = accel_read_measurement(&new_meas);
    printf("meas code: %d, meas x: %d, meas y: %d,meas z: %d\n", meas_code, new_meas.a_x_counts,new_meas.a_y_counts,new_meas.a_z_counts);
    if(queue_enqueue(&data_queue, new_meas) != 0) 
    {
        printf("Enqueue Error, Queue Full!\n");
    }
    sleep(1);
}


void data_processing_task(TcpConfig tcp_config)
{
    AccelMeasurement new_meas;
    if(queue_dequeue(&data_queue,&new_meas) != 0) 
    {
        printf("Dequeue Error, Queue Empty!\n");
        return;
    }
    
    uint32_t accel_magnitude = calculate_accel_magnitude(new_meas.a_x_counts, new_meas.a_y_counts, new_meas.a_z_counts);
    printf("Accel Magnitude: %u\n",accel_magnitude);
    uint8_t bytes_to_send[100];
    memcpy(bytes_to_send,&accel_magnitude,sizeof(accel_magnitude));
    tcp_send(&tcp_config, bytes_to_send, sizeof(accel_magnitude));
}


