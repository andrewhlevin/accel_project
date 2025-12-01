from shared_queues import logging_queue
import struct
import time
import json
import os
from typing import Dict

def logging_thread(json_file_path: str):
    directory = os.path.dirname(json_file_path)
    if directory and not os.path.exists(directory):
        # Create directory if it doesnt exist
        os.makedirs(directory, exist_ok=True) 
    with open(json_file_path, 'w') as f:
        while True:
            try:
                accel_data = logging_queue.get()  # Blocks if queue is empty
                # Append message to JSON file
                json.dump(accel_data, f)
                f.write('\n')  # one JSON object per line
                logging_queue.task_done()
                time.sleep(0.01)
            except Exception as e:
                print(f"Error in logging: {e}")
                f.close()
                return