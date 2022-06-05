---
sidebar_position: 43
title: "Lab: Building A Root Filesystem"
---

:::danger Incomplete

This document is not yet written.

:::

While a toolchain is all you need to write pure bare metal C/C++ code, often you'll want a standard library. A standard library is almost always a requirement when attempting to build open source packages or external tools for a target device. This section aims to demonstrate:

- The relationship between Linux Kernel Headers and the Standard C Library.
- The dependencies and build process to build a bare minimum build-time system root environment and runtime root filesystem.

To summarize the plan, we're going to build a fully boot-able system that will run within a QEmu emulation. For this, we're going to build our own `libc`, kernel, and user space. Other than those three components, we'll need the qemu emulator and cross compiler/toolchain. It is also worth mentioning that we're drastically simplifying this build by making `busybox` static. This means that we won't have a dynamic linker or shared object requirements for the resulting root filesystem.

## Build Environment

Its always good to have some conventions in place for file system hierarchy. For our setup, we'll have a top level `/projects` folder that contains our various exercises and experiments. Hence, the prefix for everything I'll be doing is `/projects/arm64sys`. Of course you can change this to whatever you want, just keep the substitution in mind as you follow along.

### Components

- [musl 1.2.1](https://musl.libc.org/releases/musl-1.2.1.tar.gz) - This will be used as the libc. It is similar to `uclibc`, where it is a minimal libc that is much smaller than glibc and can be commonly found in embedded systems. (~2 mins)

- [Linux 4.14](https://cdn.kernel.org/pub/linux/kernel/v4.x/linux-4.14.213.tar.xz) - This is our kernel. In this example we'll be using Linux kernel 4.14 (an LTS version). (~20 mins)

- [busybox 1.32.0](https://busybox.net/downloads/busybox-1.32.0.tar.bz2) - This is a one stop shop for nearly everything we need in the user space. The largest need this tool fills is that it acts as the shell we'll use to interact with our system and user space.

### Dependencies

<!-- TODO: Consider using the buildroot toolchain. -->

I'm using Ubuntu 20.04. In the case of using Ubuntu 20.04 there are a number of dependencies listed below that you'll want to install. Of course you could always build your own, but then you'd just be doing the same thing buildroot does in their process:

- `DEBIAN_FRONTEND=noninteractive` - Less of a dependency and more of a argument required to skip over some tzdata setup. This argument is optional in interactive environments, but must be supplied if you plan to integrate this process into a `docker build` or other automated process.
<!-- - `crossbuild-essential-arm64` - This is the ubuntu provided GNU cross compiler binary package for arm64 architectures. -->
- `libncurses5-dev` - Both busybox and linux use the Kbuild build system to configure their packages. You can opt to use a terminal only mode, but it is often recommended to use the curses mode or X mode. To use the curses configuration menu you must install the curses development package.
- `bc` - This is a build dependency of the linux kernel.
- `wget` - I used `wget` to fetch all of the archives of the components mentioned above. You could just as easily use `curl` or a web browser if you're using X.
- `fakeroot` - Fake root is the tool used to allow the creation of device nodes and system partitions (for use in VMs and emulators) as an unpriviledged user. These operations typically require root priviledges.
- `vim` & `vim-common` - As a vim user I always install these two packages as my standard text editor. Install or use what ever text editor brings you joy.
- `qemu-user-static` - This is the userspace emulator that allows us to run binaries built for other architectures in our own environment. We use the static version of this emulator so there are no dependencies on shared objects in case we want to `chroot` with it at any point in time.
- `qemu-system-arm` - This is the full system emulator where the code is completely ignorant of our host system.
- `cpio` - CPIO is the tool required to package up initramfs images read by the kernel at boot.

All of these dependencies can be run as two `apt-get` commands:

<!-- ```sh
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get install -y crossbuild-essential-arm64 \
    libncurses5-dev bc cpio fakeroot vim vim-common qemu-user-static qemu-system-arm
``` -->

<!-- TODO: Mention moving the buildroot toolchain path somewhere better? -->

```sh
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get install -y \
    libncurses5-dev bc cpio fakeroot vim vim-common qemu-user-static qemu-system-arm
```

## Building the Kernel

Building the kernel is largely independent of everything else and therefore we'll just get that out of the way. By building the kernel, we'll also learn if our cross compiler is configured correctly.

```sh
mkdir linux_build
tar -xpf linux-4.14.213.tar.xz
pushd linux-4.14.213
make ARCH=arm64 O=../linux_build defconfig
cd ../linux_build
make ARCH=arm64 menuconfig
```

Configuration Settings:

- Uncheck *Enable loadable module support*.
- General Setup -> Set *Cross-compiler tool prefix* to `aarch64-buildroot-linux-musl-`.

After the changes are updated, exit and save the configuration.

```sh
make ARCH=arm64 -j8
popd
```

With my current setup, this build of the Linux kernel tool ~10d minutes.

### The Kernel Headers and Libc Relationship

The gist is that almost all of the kernel's services are accessed with the syscall interface. This is normally the only stable API in the actual kernel. Within the kernel the APIs can completely change from release to release, but the gateway between user space and the kernel remains stable.

Its because of this stability that other libraries (e.g. uclibc, musl, and glibc) can develop a standard C interface around it that last for years and now decades. While this stability does exist, it has gone through generational change that still effects embedded systems found in the wild today. 

Linux 2.0, 2.2, 2.4, and 2.6 have subtlety different syscall interfaces. In general, Linus has tried to keep the syscall numbers all the same from release to release, but there have been some notable consolidations such as the network interface changed between 2.4 and 2.6. This means that a binary built with a compiler and libc intended for a Linux 2.6 is not likely to work with Linux 2.4.

Also, newer linux kernel headers will include system call interfaces that don't work with older kernels. This is because newer headers include all of the new syscalls. To keep maximum compatibility with a linux kernel, it's advisable to have the oldest set of kernel headers that support the required architecture.

<!-- TODO: Show the network interfaces?. -->
<!-- TODO: Review system calls?. -->

## Building sysroot and rootfs

Within our build environment, we'll actually be building two different environments. The `sysroot` environment contains all of the files required to build files from source code against our kernel and libc. The `rootfs` environment will contain all the files required to run our user space within the target system. In other words, `sysroot` is required for building things, `rootfs` is required for running things.

We start by creating the two folders:

```
mkdir -p /projects/minsys/sysroot /projects/minsys/rootfs
```

Then we'll extract, configure, make, and install the musl headers and files into the sysroot folder.

<!-- TODO: The following assumes the PATH is set correctly. -->

```sh
tar -xpf musl-1.2.1.tar.gz
pushd musl-1.2.1
CROSS_COMPILE=aarch64-buildroot-linux-musl- ./configure \
  --enable-wrapper=gcc --target=aarch64-linux-gnu \
  --prefix=/projects/minsys/sysroot
CROSS_COMPILE=aarch64-buildroot-linux-musl- make install
# Build took ~2 minutes
popd
```

After musl is built and installed, there is still a number of system specific header files that need to be included in the sysroot to build busybox. The linux headers are where headers that start with `asm/` or `bits/` come from. Note: You can't just point your `-I` arguments at the kernel directory or copy the headers from the kernel tree because as part of the `headers_install` target, a number of header file locations are adjusted or generated.

```sh
pushd linux_build
make headers_install ARCH=arm64 INSTALL_HDR_PATH=/projects/minsys/sysroot
# Build took <1 min
popd
```

Finally we'll start to run through the busybox build. Start by extracting and configuring busybox. Note: There are some expected configurations that you should use below. Its also worth nothing that there are a number of features of busybox that aren't inherently supported with our minimal environment. For example, the SCSI Eject capability will not build in our environment so you may need to disable that as part of your configuration.

Unpack busybox:

```sh
tar -xpf busybox-1.32.0.tar.bz2
pushd busybox-1.32.0
make menuconfig
```

Setup some build configurations:

- Enable `Settings -> Build Options -> Build static binary (no shared libs)`
- Set `Settings -> Build Options -> Cross compiler prefix` to `aarch64-buildroot-linux-musl-`
- Set `Settings -> Build Options -> Path to sysroot` to `/projects/playground/minsys/sysroot`
- Set `Settings -> Build Options -> Additional CFLAGS` to `-Wno-undef -Wno-parentheses -Wno-strict-prototypes -specs=/projects/minsys/sysroot/lib/musl-gcc.specs`
- Set `Settings -> Installation Options -> Destination path for 'make install'` to `/projects/minsys/rootfs`
- Disable `Linux System Utilities -> eject -> Scsi Support`

Note: Instead of dealing with the nasty menuconfig interface when you want to modify long entries like `Additional CFLAGS`, you can modify `.config` directly with a text editor. The `Additional CFLAGS` setting is found in the `CONFIG_EXTRA_CFLAGS` variable.

After you've configured busybox with your settings, build and install to the rootfs:

```sh
make install
# Build took <1min
```

Note: To see more verbose output (i.e. the commands being run), use `make V=1`.

Note: To troubleshoot busybox linking you must enabled the busybox `scripts/trylink` debug mode. You can accomplish this by setting the `debug` variable from `false` to `true` in the script (~ line 3).

Note: If you've been working with busybox before attempting these instructions, you need to remove references to `-nostdinc` and `-nostdlib` because these override the settings found in the musl specfile.

Congratulations, you should now have a working busybox static binary! Although we plan to build a complete system that boots from our kernel, at this point you can test the busybox build with the userspace qemu emulator:

```sh
qemu-aarch64-static ./busybox sh
```

To finish up with busybox build, lets popd back up to the top of the project:

```sh
popd
```

## Stage target system initial filesystem (i.e. initramfs):

There are a number of ways to boot into a linux kernel. Initramfs is commonly used in modern linux distributions. It is a file that is either embedded in the kernel image (via a linker) or available adjacent to the kernel (from some accessible media or memory). It is loaded completely into volatile memory and its purpose is to start up system required hardware devices and drives. Usually within an initramfs, the system will *pivot* the root of the system to another folder on a persistent disk. In our case, we're going to simply use initramfs as a method to boot our system into a volatile state. This way, all changes that we make to the environment will be wiped upon reboot. This is convenient for deterministically knowing the state of the machine on boot, whereas on a persistent system the reboot state can change on each reboot.

For the kernel to pass control to the user space, it attempts to execute an `init` process as the first process. You can see this in the kernel source file: `init/main.c:kernel_init()`. The following is the relevant snippet that shows the order of precedence for the lookup of a `init` binary.

```c
static char *ramdisk_execute_command = "/init";

/* ... */

if (ramdisk_execute_command) {
  ret = run_init_process(ramdisk_execute_command);
  if (!ret)
    return 0;
  pr_err("Failed to execute %s (error %d)\n",
          ramdisk_execute_command, ret);
}

/* ... This is the init= from command line. ... */

if (execute_command) {
  ret = run_init_process(execute_command);
  if (!ret)
    return 0;
  panic("Requested init %s failed (error %d).",
        execute_command, ret);
}

/* ... This is optionally sourced from .config at build time. ... */

if (CONFIG_DEFAULT_INIT[0] != '\0') {
  ret = run_init_process(CONFIG_DEFAULT_INIT);
  if (ret)
    pr_err("Default init %s failed (error %d)\n",
            CONFIG_DEFAULT_INIT, ret);
  else
    return 0;
}

/* ... Legacy locations for implicit init locations. ... */

if (!try_to_run_init_process("/sbin/init") ||
    !try_to_run_init_process("/etc/init") ||
    !try_to_run_init_process("/bin/init") ||
    !try_to_run_init_process("/bin/sh"))
  return 0;
```

Our plan is to hit the first condition by adding a `/init` to the root of our initramfs. Create `rootfs/init` script with your favorite code editor:

```sh
#!/bin/sh

mount -t proc proc /proc
mount -t sysfs proc /sys

mount -n -t tmpfs none /dev
mknod -m 622 /dev/console c 5 1
mknod -m 666 /dev/null c 1 3
mknod -m 666 /dev/zero c 1 5
mknod -m 666 /dev/ptmx c 5 2
mknod -m 666 /dev/tty c 5 0
mknod -m 444 /dev/random c 1 8
mknod -m 444 /dev/urandom c 1 9

exec /bin/sh
```

Configure the script to be executable and copy some other critical files:

```sh
pushd rootfs
chmod +x init
popd
```

There are some files that we want to create that need to be owned and created by the root user (notably the character devices `/dev/console` and `/dev/null`). To accomplish this without `sudo`, we'll use the `fakeroot` tool. Start by creating the following `build_initramfs.sh` script.

<!-- TODO: Get the non-sudo equivalents. -->

```sh
#!/bin/bash
pushd rootfs
# Note: busybox may have created some of these during its install.
mkdir -p dev bin sbin etc proc sys usr/bin usr/sbin
mknod -m 622 ./dev/console c 5 1
mknod -m 666 ./dev/null c 1 3
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../initramfs.cpio.gz
popd
```

Run the script with fakeroot for everything to run in a simulated root environment.

```sh
chmod +x build_initramfs.sh
fakeroot ./build_initramfs.sh
```

Now we can test our fully emulated system to see if it boots:

```sh
qemu-system-aarch64 -M virt -m 2048 -smp 1 -cpu cortex-a72 -no-reboot -nographic \
  -kernel linux_build/arch/arm64/boot/Image \
  -append "console=ttyAMA0 init=/init" \
  -initrd initramfs.cpio.gz
```

<!-- TODO: This isn't working from docker. -->

If everything went to plan, it should drop you into a root shell:

```text
/ #
```

Note: To exit the emulator: `Ctrl-a` then `x`

<!-- TODO: Figure out how to manage the extra stuff below. -->

## Extra Stuff

The next step is take our volatile system and make it into a non-volatile system so that when we make changes from within the emulation they will stick. Just like before, we want to create a `build_rootfs.sh` script that we'll run with the `fakeroot` tool. The following script will create an ext3 disk image that is intended to max out at 32 megabytes.

```sh
#!/bin/bash
truncate --size=32M disk.ext3
pushd rootfs
mknod -m 622 ./dev/console c 5 1
mknod -m 666 ./dev/null c 1 3
popd
mkfs.ext3 -d ./rootfs disk.ext3
```

Now create the Ext3 disk image by running the script with `fakeroot`:

```sh
chmod +x build_rootfs.sh
fakeroot ./build_rootfs.sh
```

Now we can test our fully emulated system to see if it boots:

```sh
qemu-system-aarch64 -M virt -m 2048 -smp 1 -cpu cortex-a72 -no-reboot -nographic \
  -hda disk.ext3 \
  -kernel linux_build/arch/arm64/boot/Image \
  -append "console=ttyAMA0 root=/dev/vda init=/init"
```

**TODO: This isn't working from docker.**

If everything went to plan, it should drop you into a console:

```text
/ #
```

Note: To exit the emulator: `Ctrl-a` then `x`

Note: If you don't include the `init=/init` in the kernel parameters, the kernel may attempt to run `/linuxrc` in the non-volatile (or a classical initrd) environment. If `/linuxrc` was executed, it'd show up as something similar to the following in the console:

```text
can't run '/etc/init.d/rcS': No such file or directory

Please press Enter to activate this console.
/ #
```

This works as a userspace environment as well, but it simply doesn't run the code we intended to have run in our own `init` script and therefore the behavior may be completely unknown.

## Conclusion

In building our minimal system we built our own kernel followed by a sysroot with linux headers and the musl libc headers and executable code. Using the sysroot, we were able to build a static busybox and initialize the busybox-based rootfs. Finally, some additional scripting was created and executed to setup a minimal file system hierarchy for the kernel to initalize the system and pass control to busybox for user interaction.

Once the base system was setup we were able to startup the qemu system emulator to boot the system in a volatile mode (where everything is wiped on reboot) and non-volatile mode (where writes on the ext3 filesystem are persistent).

Its worth nothing that the entire process didn't require any sudo commands (except for the potential need to `apt-get` various standard Ubuntu packages and build tools). Additionally, the whole build process takes significantly less time to build than the buildroot process.

# Notable Resources

- [How do I build a Busybox-based system?](https://busybox.net/FAQ.html#build_system)
- [Build and run minimal Linux / Busybox systems in Qemu](https://gist.github.com/chrisdone/02e165a0004be33734ac2334f215380e)
- [LFS - Populating /dev](https://tldp.org/LDP/lfs/LFS-BOOK-6.1.1-HTML/chapter06/devices.html)
- [QEMU: /bin/sh: can't access tty; job control turned off](https://stackoverflow.com/questions/36529881/qemu-bin-sh-cant-access-tty-job-control-turned-off)
- [How to create a file system as a non root user?](https://unix.stackexchange.com/questions/423965/how-to-create-a-file-system-as-a-non-root-user)
- [Exporting kernel headers for use by userspace](https://www.kernel.org/doc/html/latest/kbuild/headers_install.html)

# Other stretch goals:

- Install `gdbserver` to run from within the service to allow application level debugger. Note: When doing emulation within an emulator, the external gdb client should be the gdb-multiarch variant of gdb.
- To perform some simple runtime analysis on various processes from within the emulated environment you should build `strace` for the environment. This will display in real time all the syscalls that are used by the inspected application.
- Finally, `dropbear` is a SSH daemon. This allows the user to break away from depending on the qemu console to interact with the system. Additionally, having an SSH daemon gives the user an avenue for transferring file to and from the emulated system. This can allow one to avoid having to rebuild the whole rootfs just to update an executable. (This will likely require an SSL library to be built as well.)

## Note To Self

When I get time to adjust this ... In hindsight, it makes more sense to build the minsys folder first and then keep all built objects there. For example, first build minsys and then make sure it contains the kernel `Image`, the non-volatile `disk.ext3`, and the volatile `initramfs.cpio.gz`. It will also contain the `build_rootfs.sh`, `build_initramfs.sh`, `rootfs`, `sysroot` folders and scripts. Then from this single directory we can execute our fakeroot and qemu commands without any directory traversal. We should also be able to generate all of this from a single `docker build` command.
