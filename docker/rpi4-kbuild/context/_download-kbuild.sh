#!/bin/sh +x

mkdir {build,init}

# Clone linux source.
git clone https://github.com/raspberrypi/linux src

cd src

# Checkout known good commit.
git reset 84db2c69d8dfaa3dfcc48efaff43b562fc58fa11 --hard

# Initialize build folder.
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} O=../build bcm2711_defconfig

