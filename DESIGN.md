Firmware Design

High Level:
- Goal was to build a low latency, low memory overhead, data pipeline from mock accel interface input to TCP socket output
- Written in C (as required by the prompt)

Mock I2C Device:
- Based on given API, built a mock i2c accel device that would properly respond to driver API requests from the MCU firmware
- Emulates register reads and writes that are required by the accelerometer interface as written 
    (does not cover all cases like reading individual accel registers)
- Checks for correct device and register addresses whenever a read is performed, returns error codes if invalid address is requested
- Data is modeled by different period/amplitude sine waves for each axis, which allowed me to easily validate rough data correctness (and is cool to watch!)
- The float-defined data model is converted to 12-bit Signed, left-justified data (as requested) whenever those data registers are read
- The Data uses all 12 bits (sensor count range -2047 to 2048) to represent a physical range of -5 to 5 g, yielding a count sensitivity of 2.44 mg/count
- Implemented the stretch goal of a simulated i2c bus fault, that occurs whenever the incrementing step counter hits the unlucky number 13

Accelerometer Interface/Driver:
- Wraps Mock I2C API to allow for 2 operations: Initialize and Read All Accel Registers (only time accel should be written to is upon initialization to enable control and HPF bits)
- Initialization performs write operations to configure HPF (not implemented yet) and enable sampling, checks for SUCCESS ret codes from the Mock I2C device
- In the future, I would have added the mechanism for the client to stop and start the accel streaming on demand
- Reading only allows a request for all 6 accel registers at the same time, as I am making an assumption they are sampled (and made query-able) by the i2c device at the same time/rate (individual axes could be enabled/disabled individually as a future improvement)
- The read function then combines and returns sensor counts as int16s in the AccelMeasurement data struct (reverses the 16 to 12 bit conversion done by the mock i2c device)
- Reading also returns a RET CODE that can be used by the application to appropriately handle errors

Data Queue:
- Developed a super simple queue library without rollover
- Can enqueue, dequeue, and check if queue is full/empty
- In the future, I would enable circular behavior with rollover to allow for higher throughput, lossy data
- Given relative rates/bandwidth of processes, queue does not need to be large to ensure lossless transmission 

TCP Interface:
- Versatile, general application library for TCP socket initialization, sending, and receiving
- Entirely decoupled from the accel data we are using it for in this application (I wanted this library to be as light-weight, and reusable as possible)
- In addition to init, send, and receive methods, I added a basic payload wrapper with header bytes, sample count, and a crc to ensure data integrity and synchronization (probably overkill but would be necessary for protocols with less built-in integrity checks like RS422)
- Will wait/block forever until a client connects for the first time (Could add timeout support in the future if the system had another way for a user to restart the connection attempt)
- Does not handle dropped client connection, but would add multi-client support and connection retry in the future
- Assuming little-endian transmission of data from server to client

Main Application:
- Performs initialization of the system, then starts a bare-metal-esque super loop with 2 threads
- Thread 1: ISR thread that simulates a hardware timer interrupt (using sleeps) and reads new accel data
- Thread 2: Processing Thread that processes, packages, and sends new accel data to the client
- Every 4 ISR thread events, we sample from the i2c device as the prompt mentions that the accel sample rate is 1/4th the timer interrupt rate
- Every 4th call, we perform a mock i2c step and service the accel in 1 of 2 ways
- If the last read call did NOT return a BUS ERROR, we read again
- If the last read call DID return a BUS ERROR, we reinitialize the bus (my attempt to fulfil the stretch goal of simulated i2c bus error handling)
- The reason we don't both reinitialize and read in the same cycle is that we want to ensure as much schedule determinism as possible and i2c driver calls likely take the most time of any operation in this application. This way we gurantee 1 driver call per cycle
- Data timestamping happens as soon as processing thread receives a new sample (low latency without spending too much time in the ISR, keep the ISR as single responsinility as possible)

Client Application Design:

High Level:
- Written in Python (This is the language I have the most experience in outside of C and C++)
- Easier than other languages to spin up an app like this due to vast package support including matplotlib
- Lower performance than other languages so would likely want to use something else if data throughput requirements are high enough
- Application has 4 threads: TCP Interface, Data Parsing/Processing, Logging and Main (plotting)
- Plotting is in main due to matplotlib bandwidth needs

TCP Interface:
- Receives and verifies (checks header and CRC) accel data from server
- Handles server (firmware) restarts, and performs connection retries
- Passes data via queue to other threads for payload data processing

Accel Data Processor:
- Dequeues and Parses (unpacks) Raw Bytes from TCP Interface into accel data dictionary
- Passes Parsed Data to Logging and Plotting Queues

Logger:
- Creates and writes to JSON log file in configured location
- JSON chosen because its easily readable for development 
- Would likely use binary format instead for compactness and write speeds if data bandwidth requirements are high enough

Plotter (main):
- Using matplotlib because of its versatility and (based on my experience) decent reliability compared to other packages like tkinter
- Currently plots all accelerations on 1 plot (would break this up into multiple per axis in the future)
- Uses local timestamp instead of device timestamp for plotting in order to support firmware/mcu_clock restart

What I would do next with more time
- In addition to the ways I would improve this above, here is what I would do with more time:
- Enable High Pass filtering with configurable cutoff frequency
- Add ability to start and stop accel sampling while firmware is streaming, controllable through the client application via cmd line
- If supported by the MCU, enable DMA-based, non-blocking i2c read calls, allowing the cpu to perform other tasks while the i2c interface is waiting for bytes to be received over the bus
- Build in/incorporate RTOS-like schedule enforcement (If a blocking i2c read call does not return in X ms, yield the kernel and allow another task to takeover) 
- Add a watchdog/exception handling to handle task overruns or system hard faults

AI (ChatGPT) Uses:
- Plot initialization and data update template (Python): Reformatted it for main loop, modified timestamp source
- Accel Data Unpack function (Python): Made small tweaks to names and 
- Modbus CRC 16 function (C and Python): Used pretty much as generated
- Integer Sqrt function (C): Used pretty much as generated
- TCP Utility Code (C): Based on manually generated header, made small modifications to intilization and send functions

