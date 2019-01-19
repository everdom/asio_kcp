DIR=`pwd`
cd ${DIR}/g2log && mkdir build ; cd build && cmake .. && make
cd ${DIR}/gmock-1.7.0 && ./configure && make

cd ${DIR}/muduo
CC=gcc CXX=g++ BUILD_DIR=./build BUILD_TYPE=release BUILD_NO_EXAMPLES=1 . ./build.sh
cd ${DIR}/gtest-1.7.0 && ./configure && make
cd ${DIR}/gmock-1.7.0 && ./configure && make
cd ${DIR}
