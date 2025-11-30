import socket
import time
from shared_queues import tcp_byte_queue
# -------------------------------
# TCP Receiver Thread
# -------------------------------
def tcp_receiver_thread(host="127.0.0.1", port=24912):
    
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        try:
            sock.connect((host, port))
            print(f"Connected to server {host}:{port}")
        except Exception as e:
            print(f"Failed to connect: {e}")
            return
        
        while True:
            data = sock.recv(26)
            if not data:
                print("[Receiver] Client disconnected.")
                break

            # Add received bytes to queue (drops oldest if full)
            try:
                tcp_byte_queue.put_nowait(data)
            except:
                print("Error queue full!")


            print(f"[Receiver] Received {len(data)} bytes")

        sock.close()