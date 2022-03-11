
# Notes For Assembly

## Build U-Boot Binary

### Initialize Build Configuration for RPi4

```sh
git clone https://github.com/u-boot/u-boot.git
cd u-boot
make rpi_arm64_defconfig
make menuconfig
```

### Set the following options

- Boot Options
  - Support substituting strings in boot arguments (True)

- Console
  - Support a silent console (True)
  - Only silence the U-Boot console (True)
  - Changes to the silent environment variable update immediately (True)

- Command Line Interface
  - Memory commands
    - binop (True)
    - meminfo (True)
    - ms - Memory Search (True)
    - strings - display strings in memory (True)
  - Device access commands
    - clk - Show clock frequencies (True)
    - sdram - Print SDRAM configuration information
  - Misc Commands
    - gettime - read elapsed time (True)
    - rng command (True)

- Library Routines
  - Enable hexdump
  - Enable getopt

### Run Build

```sh
CROSS_COMPILE=aarch64-linux-gnu- make
```

## Build I2C Script

Copy the following into a `challenge.ubootsh` file.

<details><summary>Challenge Script Source</summary><br />

```text
# Generate Random Challenge
random 0x23FFFFC 4
setexpr nibbles *0x023FFFFC \& 0x000000FF
setexpr nibble1 ${nibbles} / 0x10
setexpr nibble2 ${nibbles} \& 0x0000000F
setexpr expected ${nibble1} \^ ${nibble2}

print nibbles ; print nibble1 ; print nibble2 ; print expected

# set gpio config for i2c => 0xfe200000 |= 0x00000900
setexpr v1 *0xfe200000 \| 0x00000900 ; mw.l 0xfe200000 ${v1} 1

# clear gpio resistor config => 0xfe2000e4 &= 0xFFFFFF0F
setexpr v1 *0xfe2000e4 \& 0xFFFFFF0F ; mw.l 0xfe2000e4 ${v1} 1
# set gpio pull ups => 0xfe2000e4 &= 0x00000050
setexpr v1 *0xfe2000e4 \| 0x00000050 ; mw.l 0xfe2000e4 ${v1} 1

# enable i2c => 0xfe804000 = 0x00008000;
mw.l 0xfe804000 0x00008000 1
# Set address => 0xfe80400c = 0x00000020;
mw.l 0xfe80400c 0x00000020 1

# ---> Do writes
# Set DLEN to 2 => 0xfe804008 = 2
mw.l 0xfe804008 0x00000002 1

# reset fifo => 0xfe804000 |= 0x00000030;
setexpr v1 *0xfe804000 \| 0x00000030 ; mw.l 0xfe804000 ${v1} 1
# Set GPIOB to output => 0xfe804010 = 0x01, 0xfe804010 = 0x00
mw.l 0xfe804010 0x00000001 1
mw.l 0xfe804010 0x00000000 1
# reset done => 0xfe804004 = 1 << 1;
mw.l 0xfe804004 0x00000002 1
# write start => 0xfe804000 |= 0x00000080;
setexpr v1 *0xfe804000 \| 0x00000080 ; mw.l 0xfe804000 ${v1} 1

# ---> Check status
#setexpr v1 *0xfe804004 \& 0x00000002 ; print v1
sleep 0.1

# reset fifo => 0xfe804000 |= 0x00000030;
setexpr v1 *0xfe804000 \| 0x00000030 ; mw.l 0xfe804000 ${v1} 1
# Set challenge params
mw.l 0xfe804010 0x00000013 1
mw.l 0xfe804010 ${nibbles} 1
# reset done
mw.l 0xfe804004 0x00000002 1
# write start
setexpr v1 *0xfe804000 \| 0x00000080 ; mw.l 0xfe804000 ${v1} 1
# ---> Check status
#setexpr v1 *0xfe804004 \& 0x00000002 ; print v1
sleep 0.1

# ---> Do read
# Set DLEN to 1
mw.l 0xfe804008 0x00000001 1
# reset fifo
setexpr v1 *0xfe804000 \| 0x00000030 ; mw.l 0xfe804000 ${v1} 1
mw.l 0xfe804010 0x00000012 1
# reset done
mw.l 0xfe804004 0x00000002 1
# write start
setexpr v1 *0xfe804000 \| 0x00000080 ; mw.l 0xfe804000 ${v1} 1
# ---> Check status
#setexpr v1 *0xfe804004 \& 0x00000002 ; print v1
sleep 0.1

# reset fifo
setexpr v1 *0xfe804000 \| 0x00000030 ; mw.l 0xfe804000 ${v1} 1
# reset done
mw.l 0xfe804004 0x00000002 1
# read_start
setexpr v1 *0xfe804000 \| 0x00000081 ; mw.l 0xfe804000 ${v1} 1
# ---> Check status
#setexpr v1 *0xfe804004 \& 0x00000002 ; print v1
sleep 0.1
# ---> Check FIFO
setexpr v1 *0xfe804010 \& 0x0000000F

setexpr result ${v1} ; print result

if test $result -eq $expected ; then
  setenv -f silent ; setenv bootdelay 10
  echo "Admin Mode Enabled"
  echo "Execute \"run kload kboot\" to boot kernel."
else
  echo "User Mode Enabled"
  setenv bootargs "${bootargs} quiet" ; run kload kboot
fi
```

</details><br />

Run the following to compile `challenge.ubootsh` into a runnable u-boot script `challenge.scr`:

```sh
mkimage -A arm -T script -C none -n 'Challenge Script' -d challenge.ubootsh challenge.scr
```

Copy `challenge.scr` to RPi4's /boot folder (e.g. scp).

<details><summary>Misc I2C Challenge Info</summary><br />

Command to run script manually:

```sh
load mmc 0:1 ${scriptaddr} challenge.scr; source ${scriptaddr}
```

I2C1 Controller Register Offsets:

- **C**: `0xfe804000`
- **S**: `0xfe804004`
- **DLEN**: `0xfe804008`
- **A**: `0xfe80400c`
- **FIFO**: `0xfe804010`

XOR Table:

```text
0 0 0
0 1 1
1 0 1
1 1 0
```

</details>

## Build the U-Boot Initial Environment

Copy the following data to `u-boot-env.txt`.

<details><summary>U-Boot Environment Source</summary><br />

```text
arch=arm                                              
baudrate=115200
board=rpi
board_name=4 Model B
board_rev=0x11
board_rev_scheme=1
board_revision=0xB03111
boot_a_script=load ${devtype} ${devnum}:${distro_bootpart} ${scriptaddr} ${prefix}${script}; source ${scriptaddr}
boot_efi_binary=load ${devtype} ${devnum}:${distro_bootpart} ${kernel_addr_r} efi/boot/bootaa64.efi; if fdt addr ${fdt_addr_r}; then bootefi ${kernel_addr_r} ${fdt_addr_r};else bootefi ${kernel_addr_r} ${fdtcontroladdr};fi
boot_efi_bootmgr=if fdt addr ${fdt_addr_r}; then bootefi bootmgr ${fdt_addr_r};else bootefi bootmgr;fi
boot_extlinux=sysboot ${devtype} ${devnum}:${distro_bootpart} any ${scriptaddr} ${prefix}${boot_syslinux_conf}
boot_net_usb_start=usb start
boot_pci_enum=pci enum
boot_prefixes=/ /boot/
boot_script_dhcp=boot.scr.uimg
boot_scripts=boot.scr.uimg boot.scr
boot_syslinux_conf=extlinux/extlinux.conf
boot_targets=mmc0 mmc1 mmc2 usb0 pxe dhcp 
bootcmd=run distro_bootcmd
bootcmd_dhcp=devtype=dhcp; run boot_net_usb_start; run boot_pci_enum; if dhcp ${scriptaddr} ${boot_script_dhcp}; then source ${scriptaddr}; fi;setenv efi_fdtfile ${fdtfile}; setenv efi_old_vci ${bootp_vci};setenv efi_old_arch ${bootp_arch};setenv bootp_vci PXEClient:Arch:00011:UNDI:003000;setenv bootp_arch 0xb;if dhcp ${kernel_addr_r}; then tftpboot ${fdt_addr_r} dtb/${efi_fdtfile};if fdt addr ${fdt_addr_r}; then bootefi ${kernel_addr_r} ${fdt_addr_r}; else bootefi ${kernel_addr_r} ${fdtcontroladdr};fi;fi;setenv bootp_vci ${efi_old_vci};setenv bootp_arch ${efi_old_arch};setenv efi_fdtfile;setenv efi_old_arch;setenv efi_old_vci;
bootcmd_mmc0=devnum=0; run mmc_boot
bootcmd_mmc1=devnum=1; run mmc_boot
bootcmd_mmc2=devnum=2; run mmc_boot
bootcmd_pxe=run boot_net_usb_start; run boot_pci_enum; dhcp; if pxe get; then pxe boot; fi
bootcmd_usb0=devnum=0; run usb_boot
bootdelay=2
cpu=armv8
dhcpuboot=usb start; dhcp u-boot.uimg; bootm
distro_bootcmd=for target in ${boot_targets}; do run bootcmd_${target}; done
efi_dtb_prefixes=/ /dtb/ /dtb/current/
ethaddr=dc:a6:32:33:c0:02
fdt_addr=2eff3800
fdt_addr_r=0x02600000
fdt_high=ffffffffffffffff
fdtcontroladdr=3af455b0
fdtfile=broadcom/bcm2711-rpi-4-b.dtb
initrd_high=ffffffffffffffff
kernel_addr_r=0x00080000
load_efi_dtb=load ${devtype} ${devnum}:${distro_bootpart} ${fdt_addr_r} ${prefix}${efi_fdtfile}
loadaddr=0x1000000
mmc_boot=if mmc dev ${devnum}; then devtype=mmc; run scan_dev_for_boot_part; fi
scriptaddr=0x02400000
preboot=pci enum; usb start; load mmc 0:1 ${scriptaddr} challenge.scr; source ${scriptaddr}
pxefile_addr_r=0x02500000
ramdisk_addr_r=0x02700000
scan_dev_for_boot=echo Scanning ${devtype} ${devnum}:${distro_bootpart}...; for prefix in ${boot_prefixes}; do run scan_dev_for_extlinux; run scan_dev_for_scripts; done;run scan_dev_for_efi;
scan_dev_for_boot_part=part list ${devtype} ${devnum} -bootable devplist; env exists devplist || setenv devplist 1; for distro_bootpart in ${devplist}; do if fstype ${devtype} ${devnum}:${distro_bootpart} bootfstype; then run scan_dev_for_boot; fi; done; setenv devplist
scan_dev_for_efi=setenv efi_fdtfile ${fdtfile}; for prefix in ${efi_dtb_prefixes}; do if test -e ${devtype} ${devnum}:${distro_bootpart} ${prefix}${efi_fdtfile}; then run load_efi_dtb; fi;done;run boot_efi_bootmgr;if test -e ${devtype} ${devnum}:${distro_bootpart} efi/boot/bootaa64.efi; then echo Found EFI removable media binary efi/boot/bootaa64.efi; run boot_efi_binary; echo EFI LOAD FAILED: continuing...; fi; setenv efi_fdtfile
scan_dev_for_extlinux=if test -e ${devtype} ${devnum}:${distro_bootpart} ${prefix}${boot_syslinux_conf}; then echo Found ${prefix}${boot_syslinux_conf}; run boot_extlinux; echo SCRIPT FAILED: continuing...; fi
scan_dev_for_scripts=for script in ${boot_scripts}; do if test -e ${devtype} ${devnum}:${distro_bootpart} ${prefix}${script}; then echo Found U-Boot script ${prefix}${script}; run boot_a_script; echo SCRIPT FAILED: continuing...; fi; done
serial#=100000006f8e0b2e
soc=bcm283x
stderr=serial,vidconsole
stdin=serial,usbkbd
stdout=serial,vidconsole
usb_boot=usb start; if usb dev ${devnum}; then devtype=usb; run scan_dev_for_boot_part; fi
usbethaddr=dc:a6:32:33:c0:02
vendor=raspberrypi
bootargs=coherent_pool=1M 8250.nr_uarts=1 snd_bcm2835.enable_compat_alsa=0 snd_bcm2835.enable_hdmi=1 bcm2708_fb.fbwidth=0 bcm2708_fb.fbheight=0 bcm2708_fb.fbswap=1 smsc95xx.macaddr=DC:A6:32:33:C0:02 vc_mem.mem_base=0x3ec00000 vc_mem.mem_size=0x40000000 console=ttyS0,115200 console=tty1 root=PARTUUID=42842715-02 rootfstype=ext4 maxcpus=4 elevator=deadline fsck.repair=yes rootwait maxcpus=4
kernel_comp_addr_r=0x0a000000
kernel_comp_size=0x40000000
kload_old=load mmc 0:1 $fdt_addr_r bcm2711-rpi-4-b.dtb ; load mmc 0:1 $kernel_addr_r ORIGINALkernel8.img
kload=setenv fdt_addr_r 0x2eff3800 ; load mmc 0:1 $kernel_addr_r ORIGINALkernel8.img
kboot=booti $kernel_addr_r - $fdt_addr_r
```

</details><br />

Compile the `u-boot-env.txt` into a FAT file system for u-boot to read at runtime:

```sh
mkenvimage -s 16384 -o uboot.env u-boot-env.txt
```

Copy `uboot.env` to RPi4's /boot folder (e.g. scp).

## I2C Challenge Circuit Test Application

<details><summary>Test App Source Code</summary><br />

```c
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define GPIO_BASE (0xfe200000)
// Pin configuration
#define GPFSEL0 (0/sizeof(uint32_t))
// Set output value
#define GPSET0 (0x1c/sizeof(uint32_t))
// Clear output
#define GPCLR0 (0x28/sizeof(uint32_t))
// Check level
#define GPLEV0 (0x34/sizeof(uint32_t))
// Pullup/Pulldown configuration
#define GPIO_PUP_PDN_CNTRL_REG0 (0xe4/sizeof(uint32_t))

// Broadcom calls I2C the Broadcom Serial Controller (BSC)
#define BSC1_BASE (0xfe804000)
// Easier to read alias.
#define I2C1_BASE (BSC1_BASE)
// Control register
#define I2C_C (0/sizeof(uint32_t))
// Status register
#define I2C_S (0x04/sizeof(uint32_t))
// Packet Length register
#define I2C_DLEN (0x08/sizeof(uint32_t))
// Address register
#define I2C_A (0x0c/sizeof(uint32_t))
// 8-bit wide data fifo register
#define I2C_FIFO (0x10/sizeof(uint32_t))

#define I2C_MODE_BYTE (1)

volatile uint32_t * mmap_gpio()
{
  int gpioctrlfd = open("/dev/mem", O_RDWR | O_SYNC);

  if (gpioctrlfd == -1) {
    printf("error opening /dev/mem.\n");
    exit(0);
  }

  volatile uint32_t *gpioctrl = (volatile uint32_t *)mmap((void *)GPIO_BASE, 0x100, PROT_READ | PROT_WRITE, MAP_SHARED, gpioctrlfd, GPIO_BASE);
  if (gpioctrl == (void *)-1) {
    printf("Failed to mmap.\n");
    exit(0);
  }

  return gpioctrl;
}

void init_gpio(volatile uint32_t *gpioctrl)
{
  // Clear config for GPIO 2 and 3 (pin 3 and 5)
  gpioctrl[GPFSEL0] &= ~0x00000FC0;
  // Set GPIO 2 and 3 to Alt Func 0: SDA1/SDC1 (pin 3 and 5)
  gpioctrl[GPFSEL0] |= 0x00000900;

  // Clear GPIO 2 and 3 resistor config.
  gpioctrl[GPIO_PUP_PDN_CNTRL_REG0] &= ~0x000000F0;
  // Set GPIO 2 and 3 pullup resistor config.
  gpioctrl[GPIO_PUP_PDN_CNTRL_REG0] |= 0x00000050;

  printf("GPFSEL0 (0x%08x) = 0x%08x\n", &gpioctrl[GPFSEL0], gpioctrl[GPFSEL0]);
  printf("PUP_PDN (0x%08x) = 0x%08x\n", &gpioctrl[GPIO_PUP_PDN_CNTRL_REG0], gpioctrl[GPIO_PUP_PDN_CNTRL_REG0]);
  printf("GPLEV0 (0x%08x) = 0x%08x\n", &gpioctrl[GPLEV0], gpioctrl[GPLEV0]);
}

void i2c_enable(volatile uint32_t *i2cc)
{
  i2cc[I2C_C] = 0x00008000;
}

void reset_fifo(volatile uint32_t *i2cc)
{
  // Clear FIFO
  i2cc[I2C_C] |= 0x00000030;
}

uint32_t fifo_empty(volatile uint32_t *i2cc)
{
  return (i2cc[I2C_S] >> 6) & 0x1;
}

uint32_t xfer_done(volatile uint32_t *i2cc)
{
  return (i2cc[I2C_S] >> 1) & 0x1;
}

uint32_t xfer_active(volatile uint32_t *i2cc)
{
  return i2cc[I2C_S] & 0x1;
}

uint32_t xfer_error(volatile uint32_t *i2cc)
{
  return (i2cc[I2C_S] >> 8) & 0x1;
}

uint32_t fifo_len(volatile uint32_t *i2cc)
{
  return i2cc[I2C_DLEN] & 0xFFFF;
}

void reset_done(volatile uint32_t *i2cc)
{
  i2cc[I2C_S] = 1 << 1;
}

void write_start(volatile uint32_t *i2cc)
{
  i2cc[I2C_C] |= 0x00000080;
}

void read_start(volatile uint32_t *i2cc)
{
  i2cc[I2C_C] |= 0x00000081;
}

void xfer_block(volatile uint32_t *i2cc)
{
  while (xfer_done(i2cc) != 1) {;}
}

void xfer_mode(volatile uint32_t *i2cc, uint8_t mode)
{
  i2cc[I2C_DLEN] = mode;
}

void i2cset(volatile uint32_t *i2cc, uint8_t chip_addr, uint8_t reg_addr, uint8_t value)
{
  i2cc[I2C_A] = (uint32_t)chip_addr;
  xfer_mode(i2cc, 2);

  // Write Register Address
  reset_fifo(i2cc);
  i2cc[I2C_FIFO] = (uint32_t)reg_addr;
  i2cc[I2C_FIFO] = (uint32_t)value;
  reset_done(i2cc);
  write_start(i2cc);
  xfer_block(i2cc);
}

uint32_t i2cget(volatile uint32_t *i2cc, uint8_t chip_addr, uint8_t reg_addr)
{
  i2cc[I2C_A] = (uint32_t)chip_addr;
  xfer_mode(i2cc, I2C_MODE_BYTE);

  // Write Register Address
  reset_fifo(i2cc);
  i2cc[I2C_FIFO] = (uint32_t)reg_addr;
  reset_done(i2cc);
  write_start(i2cc);
  xfer_block(i2cc);

  // Read Register Value
  reset_fifo(i2cc);
  reset_done(i2cc);
  read_start(i2cc);
  xfer_block(i2cc);

  return i2cc[I2C_FIFO] & 0xFF;
}

volatile uint32_t *mmap_i2cc()
{
  int i2ccfd = open("/dev/mem", O_RDWR | O_SYNC);

  if (i2ccfd == -1) {
    printf("error opening /dev/mem for i2c controller.\n");
    exit(0);
  }

  volatile uint32_t *i2cc = (volatile uint32_t *)mmap((void *)I2C1_BASE, 0x20, PROT_READ | PROT_WRITE, MAP_SHARED, i2ccfd, I2C1_BASE);
  if (i2cc == (void *)-1) {
    printf("Failed to mmap for I2C1 controller.\n");
    exit(0);
  }

  return i2cc;
}

uint8_t challenge(volatile uint32_t *i2cc, uint8_t chip_addr, uint8_t p1, uint8_t p2)
{
  //printf("Set the nibbles we want to XOR (`XOR(0xA, 0x3)`) in GPIOB\n");
  i2cset(i2cc, chip_addr, 0x13, p1 << 4 | p2);

  //printf("If everything is good, we'll get `0x09` from GPIOA\n");
  return i2cget(i2cc, chip_addr, 0x12);
}

int main(int argc, void **argv)
{
  if (argc < 3) {
    printf("Usage: %s <p1> <p2>\n", argv[0]);
    exit(1);
  }

  volatile uint32_t *gpioctrl = mmap_gpio();
  init_gpio(gpioctrl);

  volatile uint32_t *i2cc = mmap_i2cc();
  i2c_enable(i2cc);

  //printf("Set GPIOB port to all outputs\n");
  i2cset(i2cc, 0x20, 0x01, 0x00);

  errno = 0;
  uint8_t p1 = (uint8_t)strtol(argv[1], NULL, 0); //0x5;
  if (errno) {
    printf("<p1> invalid.\n", argv[1]);
    exit(1);
  }

  errno = 0;
  uint8_t p2 = (uint8_t)strtol(argv[2], NULL, 0); //0xA;
  if (errno) {
    printf("<p1> invalid.\n", argv[1]);
    exit(1);
  }

  uint8_t result = challenge(i2cc, 0x20, p1, p2);
  printf("xor(0x%02x, 0x%02x) = 0x%02x (expected 0x%02x)\n", p1, p2, result, p1 ^ p2);
  if (result == (p1 ^ p2)) {
    printf("Passed!\n");
  } else {
    printf("Failed!\n");
  }

  return 0;
}
```

</details><br />