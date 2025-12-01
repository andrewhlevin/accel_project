To Build the Firmware and run unit tests:

1. Ensure CMake and GCC are installed
2. Simply run ./BuildScript.sh (relies on gtest but fetches content from git so no previous installation required)


To Run Python Client App:

1. First install matplotlib: pip install matplotlib
2. Then run ./StartClientApp.sh

To Run Firmware App:

1. Ensure firmware is built as described above
2. Run ./StartFirmware.sh


Notes:
- Either Firmware or Client can be started first
- If firware stops or restarts, client will continue running and will function properly when firmware begins again
- If client stops or restarts, firmware will exit and would need to be restarted to regain data streaming functionality