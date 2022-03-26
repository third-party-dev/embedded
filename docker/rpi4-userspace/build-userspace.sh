#!/bin/sh

./download-userspace.sh
./build-musl.sh
./build-busybox.sh
./build-app.sh
./build-initramfs.sh