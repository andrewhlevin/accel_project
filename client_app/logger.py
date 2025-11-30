from shared_queues import logging_queue
import struct
import time
import json
from typing import Dict

def logging_thread(json_file_path: str):
    while True:
        try:
            accel_data = logging_queue.get()  # Blocks if queue is empty
            # Append message to JSON file
            with open(json_file_path, 'a') as f:
                json.dump(accel_data, f)
                f.write('\n')  # one JSON object per line

            logging_queue.task_done()
            time.sleep(0.01)
        except Exception as e:
            print(f"Error in logging: {e}")