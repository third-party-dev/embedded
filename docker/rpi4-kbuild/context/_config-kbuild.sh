#!/bin/sh +x

cd build

# Initialize build folder.
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} menuconfig

