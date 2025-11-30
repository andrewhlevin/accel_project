from queue import Queue


tcp_byte_queue = Queue(maxsize=100)
logging_queue = Queue(maxsize=100)
plotting_queue = Queue(maxsize=100)
