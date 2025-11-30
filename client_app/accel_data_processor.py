from shared_queues import tcp_byte_queue, logging_queue, plotting_queue
import struct
import time
from typing import Dict

def processing_thread():

    while True:

        if(not tcp_byte_queue.empty()):
            packet = tcp_byte_queue.get()

            if(len(packet) >= 26):
                accel_data_msg = unpack_accel_data_message(packet)
                if(logging_queue.full()):
                    print("Logging queue full!\n")
                else:
                    logging_queue.put(accel_data_msg)
                    #plotting_queue.put_nowait(accel_data_msg)
        
        time.sleep(0.1)  # 10 Hz processing loop



# Define the struct format
# '<' = little-endian (assume little-endian, adjust if needed)
# B = uint8, H = uint16, I = uint32, Q = uint64, h = int16

def unpack_accel_data_message(data: bytes) -> Dict:
    ACCEL_DATA_MESSAGE_FORMAT = '<BBI Q h h h I H'
    ACCEL_DATA_MESSAGE_SIZE = struct.calcsize(ACCEL_DATA_MESSAGE_FORMAT)

    if len(data) < ACCEL_DATA_MESSAGE_SIZE:
        raise ValueError(f"Data too short. Expected {ACCEL_DATA_MESSAGE_SIZE} bytes, got {len(data)}")

    parsed_data = struct.unpack(ACCEL_DATA_MESSAGE_FORMAT, data[:ACCEL_DATA_MESSAGE_SIZE])
    
    return {
        "header_byte_1": parsed_data[0],
        "header_byte_2": parsed_data[1],
        "sample_number": parsed_data[2],
        "timestamp_ms": parsed_data[3],
        "a_x_mg": parsed_data[4],
        "a_y_mg": parsed_data[5],
        "a_z_mg": parsed_data[6],
        "a_magnitude_counts": parsed_data[7],
        "crc": parsed_data[8]
    }