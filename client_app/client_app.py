import threading
from tcp_interface import *
from accel_data_processor import *
from logger import *

# -------------------------------
# Global shared buffer + lock
# -------------------------------
shared_payload = b""
payload_lock = threading.Lock()


# -------------------------------
# Main Entry
# -------------------------------
if __name__ == "__main__":
    # Start receiver thread
    t1 = threading.Thread(target=tcp_receiver_thread, daemon=True)
    t1.start()

    # Start processor thread
    t2 = threading.Thread(target=processing_thread, daemon=True)
    t2.start()
    
    json_file_path = "accel_data.json"
    t3 = threading.Thread(args=(json_file_path,),target=logging_thread, daemon=True)
    t3.start()

    # Keep main thread alive
    while True:
        time.sleep(5)


