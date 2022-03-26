#!/bin/sh

cd musl

CROSS_COMPILE=${CROSS_COMPILE} ./configure \
  --enable-wrapper=gcc --target=aarch64-linux-gnu \
  --prefix=/opt/boxbuild/sysroot
CROSS_COMPILE=${CROSS_COMPILE} make install

cd /opt/kbuild/build
make headers_install ARCH=arm64 INSTALL_HDR_PATH=/opt/boxbuild/sysroot
