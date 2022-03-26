# Notes

## Build Kernel

https://www.amazon.com/gp/product/B085T2NHPH
https://www.raspberrypi.com/documentation/computers/linux_kernel.html
https://github.com/raspberrypi/linux

git clone --depth=1 https://github.com/raspberrypi/linux
./run.sh
cd linux
export KERNEL=kernel8
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- bcm2711_defconfig
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- Image modules dtbs

real    65m31.488s
user    53m8.680s
sys     15m8.900s

## Start wifi without restart

https://raspberrypi.stackexchange.com/questions/85599/how-to-start-stop-wpa-supplicant-on-default-raspbian

`sudo vi /etc/wpa_supplicant/wpa_supplicant.conf`

```text
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=US

network={
     ssid="SSID"
     psk="WAP PASSWORD"
}
```

```text
$ sudo ip link set dev wlan0 down
$ wpa_cli ping
Selected interface 'p2p-dev-wlan0'
PONG
$ sudo ip link set dev wlan0 up
$ sudo systemctl daemon-reload
$ sudo systemctl restart dhcpcd.service
```

`192.168.50.118`

## Build UBoot



https://stackoverflow.com/questions/64014958/cant-get-u-boot-running-on-raspberry-pi-4b-what-are-the-absolute-necessities

make rpi_arm64_defconfig
CROSS_COMPILE=aarch64-linux-gnu- make

U-Boot> setenv bootargs coherent_pool=1M 8250.nr_uarts=1 snd_bcm2835.enable_compat_alsa=0 snd_bcm2835.enable_hdmi=1 snd_bcm2835.enable_headphones=1 smsc95xx.macaddr=DC:A6:32:05:53:B6 vc_mem.mem_base=0x3ec00000 vc_mem.mem_size=0x40000000 console=ttyS0,115200 console=tty1 root=PARTUUID=7f586f18-02 rootfstype=ext4 elevator=deadline rootwait
U-Boot> load mmc 0:1 $fdt_addr_r bcm2711-rpi-4-b.dtb                 
47471 bytes read in 26 ms (1.7 MiB/s)
U-Boot> load mmc 0:1 $kernel_addr_r kernel8.img
15483392 bytes read in 823 ms (17.9 MiB/s)
U-Boot> booti $kernel_addr_r - $fdt_addr_r

mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n "My script" -d u-boot.txt u-boot.scr

load mmc 0:1 ${scriptaddr} challenge.scr; source ${scriptaddr}

ORIGINALkernel8.img

setenv bootargs coherent_pool=1M 8250.nr_uarts=1 smsc95xx.macaddr=DC:A6:32:33:C0:02 vc_mem.mem_base=0x3ec00000 vc_mem.mem_size=0x40000000 console=ttyS0,115200 console=tty1 root=PARTUUID=42842715-02 rootfstype=ext4 maxcpus=4 elevator=deadline fsck.repair=yes rootwait maxcpus=4

load mmc 0:1 $fdt_addr_r bcm2711-rpi-4-b.dtb ; load mmc 0:1 $kernel_addr_r ORIGINALkernel8.img ; setenv kernel_comp_addr_r 0x0a000000 ; setenv kernel_comp_size 0x40000000 ; booti $kernel_addr_r - $fdt_addr_r


load mmc 0:1 $fdt_addr_r bcm2711-rpi-4-b.dtb ; fdt addr $fdt_addr_r
load mmc 0:1 0x09000000 overlays/i2c1.dtbo
fdt resize 65535
fdt apply 0x09000000

load mmc 0:1 $fdt_addr_r rpi4.dtb ; fdt addr $fdt_addr_r

### I2C Bit Bang

http://www.bitbanging.space/posts/bitbang-i2c

https://www.embeddedrelated.com/showcode/334.php

### Config Options To Consider

Support a silent console

### U-Boot Booter Memory Layout

/projects/stable/embedded/uboot/u-boot/include/configs/rpi.h

mkenvimage -s 16384 -o uboot.env u-boot-env.txt

Loading 'u-boot64-works.bin' to 0x80000 size 0x8fc98

setenv silent 1 ; run kload ; run kboot




## XOR Lock

[Phototransistor Tutorial](https://www.youtube.com/watch?v=zx4OUXk0LSQ)



## Unrelated

docker system df -v

docker pull klausmeyer/docker-registry-browser
docker run --rm --network host --name registry-browser -p 8080:8080 klausmeyer/docker-registry-browser

https://hub.docker.com/r/klausmeyer/docker-registry-browser/

https://docs.docker.com/registry/deploying/

https://gist.github.com/kekru/4e6d49b4290a4eebc7b597c07eaf61f2
https://www.docker.com/blog/how-to-deploy-on-remote-docker-hosts-with-docker-compose/

"terminal.integrated.rightClickBehavior": "copyPaste",
https://github.com/microsoft/vscode/issues/142927

https://docs.microsoft.com/en-us/sysinternals/downloads/psexec
https://docs.microsoft.com/en-us/sysinternals/downloads/



## I2C

https://www.mathworks.com/help/supportpkg/raspberrypiio/ref/add-digital-i-o-pins-to-raspberry-pi-hardware-using-mcp23017.html#:~:text=Connect%20MCP23017,-Connect%20the%20MCP23017&text=To%20power%20the%20MCP23017%2C%20connect,A0%2C%20A1%2C%20A2%20pins.

<!-- IODIRA
sudo i2cget -y 1 0x20 0x00 b

Get IODIRB
sudo i2cget -y 1 0x20 0x01 b
Set GPIOB as output
sudo i2cset -y 1 0x20 0x01 0x00 b

GPIOA
sudo i2cget -y 1 0x20 0x12 b

GPIOB
sudo i2cget -y 1 0x20 0x13 b

A - input (bottom), B - output (top)

sudo i2cset -y 1 0x20 0x13 0xA3 b

sudo i2cget -y 1 0x20 0x12 b -->

### Procedures

Check if `MCP23017` is connected, powered, and out of reset with:

```sh
sudo i2cdetect -y 1
```

On success, the device will have the address `0x20` (seen the table below.). **Note:** In this case, you can ignore the address `0x48`. If all 3 of the address pins on the `MCP23017` are not pulled to ground, it may result in another address (i.e. not `0x20`). If the address is not deliberately biased towards 3.3v or GND, the address is nondeterministic and may change in the middle of an access!

```text
pi@raspberrypi:~$ sudo i2cdetect -y 1
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: 20 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- 48 -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
```

When the system comes up, all GPIO defaults as input. Set GPIOB port to all outputs:

```sh
sudo i2cset -y 1 0x20 0x01 0x00 b
```

Set the nibbles we want to XOR (`XOR(0xA, 0x3)`) in GPIOB:

```sh
sudo i2cset -y 1 0x20 0x13 0xA3 b
```

If everything is good, we'll get `0x09` from GPIOA:

```sh
sudo i2cget -y 1 0x20 0x12 b
```

**Note:** GPIOA should be masked with `0x0F` when read.

mkimage -A arm -O linux -T ramdisk -C none -n "RootFilesystem" -d ./initramfs.cpio.gz ./initramfs.uboot

docker run -ti --rm -v $(pwd):/cfg crazychenz/rpi4-kbuild bash

https://unix.stackexchange.com/questions/72320/how-can-i-hook-on-to-one-terminals-output-from-another-terminal