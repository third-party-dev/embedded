#!/bin/sh +x

git clone https://github.com/u-boot/u-boot.git
git reset 589c659035a44a683b087fd75fe0b7667f7be7f5 --hard
cp uboot-backup.config u-boot/.config
