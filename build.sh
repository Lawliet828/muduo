#!/bin/bash
set -euo pipefail

SOURCE_DIR=`pwd`
BUILD_DIR=${BUILD_DIR:-../build}
BUILD_TYPE=${BUILD_TYPE:-release}
INSTALL_DIR=${INSTALL_DIR:-../${BUILD_TYPE}-install-cpp17}
CXX=${CXX:-g++}

ln -sf $BUILD_DIR/$BUILD_TYPE-cpp17/compile_commands.json

mkdir -p $BUILD_DIR/$BUILD_TYPE-cpp17 \
  && cd $BUILD_DIR/$BUILD_TYPE-cpp17 \
  && cmake \
           -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
           -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
           -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
           $SOURCE_DIR \
  && make -j 8

# Use the following command to run all the unit tests
# at the dir $BUILD_DIR/$BUILD_TYPE :
# CTEST_OUTPUT_ON_FAILURE=TRUE make test
