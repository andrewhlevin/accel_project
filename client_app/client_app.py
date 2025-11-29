import socket

SERVER_IP = "127.0.0.1"  # Replace with your server IP if not localhost
SERVER_PORT = 24912      # Replace with the port your server uses

def main():
    # Create TCP socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        try:
            sock.connect((SERVER_IP, SERVER_PORT))
            print(f"Connected to server {SERVER_IP}:{SERVER_PORT}")
        except Exception as e:
            print(f"Failed to connect: {e}")
            return

        while True:
            try:
                data = sock.recv(1)  # Receive up to 1024 bytes
                print(str(data))
                if not data:
                    print("Server closed connection.")
                    break
                print("Received:", data.decode('utf-8', errors='replace'))
            except KeyboardInterrupt:
                print("Client exiting.")
                break
            except Exception as e:
                print(f"Error receiving data: {e}")
                break

if __name__ == "__main__":
    main()