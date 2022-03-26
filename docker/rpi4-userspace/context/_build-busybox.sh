#!/bin/sh

cp busybox-1.35.config ./busybox/.config

cd busybox
make install