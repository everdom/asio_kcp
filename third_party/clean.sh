#!/bin/bash
CUR_PATH=`pwd`
for dir in "g2log"  "gmock-1.7.0"  "gtest-1.7.0"  "muduo"
do
    rm -rf ${CUR_PATH}/${dir}
done
