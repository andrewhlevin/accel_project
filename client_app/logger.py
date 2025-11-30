from shared_queues import tcp_byte_queue, logging_queue, plotting_queue
import struct
import time
import json
from typing import Dict

def logging_thread(json_file_path: str):
    while True:
        try:
            print("getting new log data!\n")
            accel_data = logging_queue.get()  # Blocks if queue is empty
            # Append message to JSON file
            with open(json_file_path, 'a') as f:
                json.dump(accel_data, f)
                f.write('\n')  # one JSON object per line

            logging_queue.task_done()
        except Exception as e:
            print(f"Error in accel_writer_thread: {e}")