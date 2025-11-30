from queue import Queue


tcp_byte_queue = Queue(maxsize=50)
logging_queue = Queue(maxsize=50)
plotting_queue = Queue(maxsize=1)
