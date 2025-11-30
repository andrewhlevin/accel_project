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
    static I2C_RET_CODE last_ret_code = SUCCESS;

    if(last_ret_code == BUS_ERROR)
    {
        // If received a bus error on last read call, use this schedule slot to reinit the bus
        printf("Reinitializing the bus due to error!\n");
        last_ret_code = accel_init(HPF_DISABLED);
        i2c_mock_step();
    }
    else if(down_sample_count == 3)
    {
        AccelMeasurement new_meas;
        last_ret_code = accel_read_measurement(&new_meas);

        i2c_mock_step();

        if(last_ret_code == SUCCESS)
        {
            queue_enqueue(&data_queue, new_meas); 
        }
        
        down_sample_count = 0;

    }
    else 
    {
        down_sample_count++;
    }


    usleep(10000);
}


void data_processing_task(TcpConfig tcp_config)
{
    AccelMeasurement new_meas;
    if(queue_dequeue(&data_queue,&new_meas) != 0) 
    {
        //printf("Dequeue Error, Queue Empty!\n");
        return;
    }
    
    uint8_t output_buffer[OUTPUT_MESSAGE_SIZE];
    AccelDataPayload new_payload = construct_accel_data_payload(new_meas);
    uint8_t payload_buffer[sizeof(new_payload)];
    memcpy(payload_buffer,&new_payload,sizeof(new_payload));

    if(generate_tcp_msg(payload_buffer, sizeof(new_payload), output_buffer, OUTPUT_MESSAGE_SIZE) != 0)
    {
        //printf("Generate Message Error, Buffer not large enough!\n");
        return;
    }

    tcp_send(&tcp_config, output_buffer, OUTPUT_MESSAGE_SIZE);
}


AccelDataPayload construct_accel_data_payload(AccelMeasurement meas)
{
    AccelDataPayload new_payload;
    
    new_payload.timestamp_ms = get_time_in_ms();

    new_payload.a_x_mg = (int16_t) ((float) meas.a_x_counts * MAX_SUPPORTED_Gs / 2048 * 1000);
    new_payload.a_y_mg = (int16_t) ((float) meas.a_y_counts * MAX_SUPPORTED_Gs / 2048 * 1000);
    new_payload.a_z_mg = (int16_t) ((float) meas.a_z_counts * MAX_SUPPORTED_Gs / 2048 * 1000);

    new_payload.a_magnitude_counts = calculate_accel_magnitude(meas.a_x_counts, meas.a_y_counts, meas.a_z_counts);

    return new_payload;
}

uint64_t get_time_in_ms()
{
    struct timespec spec;
    uint64_t milliseconds;

    // Get the current time with nanosecond precision
    if (clock_gettime(CLOCK_REALTIME, &spec) == -1) {
        perror("clock_gettime");
        return 1;
    }

    // Convert seconds and nanoseconds to milliseconds
    milliseconds = (uint64_t)spec.tv_sec * 1000 + (uint64_t)spec.tv_nsec / 1000000;
    return milliseconds;
}