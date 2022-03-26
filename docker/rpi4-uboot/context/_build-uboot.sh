#!/bin/sh +x

cd u-boot
#make rpi_arm64_defconfig

CROSS_COMPILE=${CROSS_COMPILE} make

# mkimage -A arm -T script -C none -n 'Challenge Script' -d challenge.ubootsh challenge.scr
# mkenvimage -s 16384 -o uboot.env u-boot-env.txt