#!/bin/bash
CUR_PATH=`pwd`
for dir in "g2log"  "gmock-1.7.0"  "gtest-1.7.0"  "muduo"
do
    cd $CUR_PATH/$dir
    mv ./$dir ${dir}_1
    mv ./${dir}_1/* ./
    rm -rf ./${dir}_1
done

