#!/bin/sh

cd app
${CROSS_COMPILE}gcc -static -o /opt/boxbuild/app-aarch64 app.c -lncurses -ltinfo
${CROSS_COMPILE}gcc -static -g -o /opt/boxbuild/app-aarch64-debug app.c -lncurses -ltinfo
cp /opt/boxbuild/app-aarch64-debug /opt/boxbuild/rootfs/app