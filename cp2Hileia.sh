#!/bin/bash
HILEIA_KCP_PATH=/media/everdom/Data/Projects/HiLeia/kcpim/src/main/cpp/kcp
BAK_PATH=/media/everdom/Data/BAK
rm -rf ${BAK_PATH}/kcp
mv ${HILEIA_KCP_PATH} ${BAK_PATH}/
mkdir ${HILEIA_KCP_PATH}
cd ./client_lib
rm *.o
rm *.a
cd ..
cp -r ./client_lib ${HILEIA_KCP_PATH}
cp -r ./essential ${HILEIA_KCP_PATH}
cp -r ./util ${HILEIA_KCP_PATH}
cp ./platform.h ${HILEIA_KCP_PATH}

