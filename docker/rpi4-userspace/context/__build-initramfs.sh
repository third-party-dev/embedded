#!/bin/bash

cd rootfs
# Note: busybox may have created some of these during its install.
mkdir -p dev bin sbin etc proc sys usr/bin usr/sbin
mknod -m 622 ./dev/console c 5 1
mknod -m 666 ./dev/null c 1 3
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../initramfs.cpio.gz
cd ..