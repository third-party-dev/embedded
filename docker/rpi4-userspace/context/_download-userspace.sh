#!/bin/sh +x

mkdir {sysroot,rootfs}

# Clone musl (i.e. libc) source.
git clone git://git.musl-libc.org/musl musl
cd musl
git checkout v1.2.2
cd ..

git clone https://git.busybox.net/busybox busybox
cd busybox
git checkout 1_35_0
cd ..

