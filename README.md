# colibri

### Prerequisites
sudo apt-get install libusb-dev libuv-dev libeigen3-dev libboost-all-dev libopencv-all-dev clang-format

#### Realsense2
https://github.com/IntelRealSense/librealsense/blob/development/doc/distribution_linux.md

git clone https://github.com/inivation/libcaer.git
mkdir build && cd build
cmake -DENABLE_OPENCV=1 ..
make
sudo make install


git clone git://github.com/ethz-asl/libnabo.git
mkdir build && cd build
cmake ..
make
sudo make install

git clone https://github.com/ethz-asl/libpointmatcher.git
mkdir build && cd build
cmake ..
make
sudo make install


#### Pangolin
sudo apt-get install ffmpeg libavcodec-dev libavutil-dev libavformat-dev libswscale-dev 
sudo apt-get install libjpeg-dev libpng-dev libtiff5-dev libopenexr-dev
sudo apt-get install libdc1394-22-dev libraw1394-dev
sudo apt-get install libpython2.7-dev
sudo apt-get install libglew-dev

git clone https://github.com/stevenlovegrove/Pangolin.git
cd Pangolin
mkdir build
cd build
cmake ..
cmake --build .

### Building instructions
```bash
git clone https://github.com/qqqzzz/colibri.git
cd colibri
git submodule update --init --recursive
mkdir build
cd build
cmake ..
make -j
```
