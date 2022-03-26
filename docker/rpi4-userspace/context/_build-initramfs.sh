#!/bin/sh +x

cp init ./rootfs/init
fakeroot ./__build-initramfs.sh
mkimage -A arm -O linux -T ramdisk -C none -n "RootFilesystem" -d ./initramfs.cpio.gz ./initramfs.uboot