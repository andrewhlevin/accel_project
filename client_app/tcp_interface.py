import socket
import time
from shared_queues import tcp_byte_queue

def tcp_receiver_thread(host="127.0.0.1", port=24912, retry_delay=2.0):
    """Continuously attempts to connect to server and enqueue received bytes."""
    while True:
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
                sock.settimeout(5)  # optional: timeout for recv/connect
                print(f"[Receiver] Trying to connect to {host}:{port}...")
                sock.connect((host, port))
                print(f"[Receiver] Connected to server {host}:{port}")

                while True:
                    try:
                        data = sock.recv(26)
                        if not data:
                            print("[Receiver] Server disconnected.")
                            break  # exit inner loop to reconnect
                        # Add received bytes to queue
                        try:
                            if(validate_accel_data_message(data)):
                                tcp_byte_queue.put_nowait(data)
                            else:
                                print("Invalid Message!\n")
                        except:
                            continue
                           #print("[Receiver] Queue full, dropping packet!")

                        #print(f"[Receiver] Received {len(data)} bytes")

                    except socket.timeout:
                        continue  # allow retry on timeout
                    except Exception as e:
                        print(f"[Receiver] Error during recv: {e}")
                        break  # exit inner loop to reconnect

        except Exception as e:
            print(f"[Receiver] Connection failed: {e}")

        print(f"[Receiver] Waiting {retry_delay}s before retrying...")
        time.sleep(retry_delay)

def validate_accel_data_message(data: bytes) -> bool:
    if(data[0] != 0x01):
        print("header 1 failed\n")

        return False
    elif(data[1] != 0x02):
        print("header 2 failed\n")
        return False
    elif(((data[25] << 8) | data[24] ) != crc16(data[:-2])):
        print("crc failed\n")
        return False
    else:
        return True




def crc16(data: bytes) -> int:
    """
    Calculate CRC-16 (Modbus style) of a bytes object.
    
    :param data: Input data as bytes
    :return: CRC16 as integer (0-0xFFFF)
    """
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x0001:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
            crc &= 0xFFFF  # Keep crc as 16-bit
    return crc
