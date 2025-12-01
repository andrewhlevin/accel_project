import threading
import matplotlib.pyplot as plt
from datetime import datetime

from tcp_interface import *
from accel_data_processor import *
from logger import *
from shared_queues import plotting_queue

if __name__ == "__main__":
    # Start TCP receiver thread
    t1 = threading.Thread(target=tcp_receiver_thread, daemon=True)
    t1.start()

    # Start Data Processor thread
    t2 = threading.Thread(target=processing_thread, daemon=True)
    t2.start()

    # Get Data Time String for log file name
    now = datetime.now()
    date_time_string = now.strftime("%Y_%m_%d_%H_%M_%S")

    # Creat log file path and start logger thread
    json_file_path = "logs/accel_data_" + date_time_string + ".json"
    t3 = threading.Thread(args=(json_file_path,),target=logging_thread, daemon=True)
    t3.start()

    # Enable interactive mode
    plt.ion()
    fig, ax = plt.subplots()

    # Lists to store data
    timestamps = []
    a_x = []
    a_y = []
    a_z = []

    # Create line objects for each axis
    line_x, = ax.plot([], [], label='a_x')
    line_y, = ax.plot([], [], label='a_y')
    line_z, = ax.plot([], [], label='a_z')

    ax.set_xlabel('Timestamp (ms)')
    ax.set_ylabel('Acceleration (mg)')
    ax.set_title('Real-Time Accelerometer Data')
    ax.legend()

    while True:
        try:
            # Get the next accelerometer message from the queue
            accel_data: Dict = plotting_queue.get()  # Blocks if queue is empty
            # Update lists with new data
            timestamps.append(time.time())
            a_x.append(accel_data["a_x_mg"])
            a_y.append(accel_data["a_y_mg"])
            a_z.append(accel_data["a_z_mg"])

            # Update line data
            line_x.set_data(timestamps, a_x)
            line_y.set_data(timestamps, a_y)
            line_z.set_data(timestamps, a_z)

            # Adjust axes limits dynamically
            ax.relim()
            ax.autoscale_view()

            # Draw updated figure
            fig.canvas.draw()
            fig.canvas.flush_events()

            # Mark the queue item as done
            plotting_queue.task_done()
        except Exception as e:
            print(f"Error in plotting thread: {e}")

            time.sleep(0.1)


