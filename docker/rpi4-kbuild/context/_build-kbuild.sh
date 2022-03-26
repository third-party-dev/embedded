#!/bin/sh

cp kbuild.config ./build/.config

cd build
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} Image modules dtbs