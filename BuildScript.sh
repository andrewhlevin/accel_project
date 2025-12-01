rm build -rf
mkdir build
cd build
cmake ../device_firmware -Wno-dev
make -j
ctest 