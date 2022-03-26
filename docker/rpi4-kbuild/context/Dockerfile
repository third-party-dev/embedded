FROM ubuntu:20.04

# Most things (e.g. bash) are better than /bin/sh.
# Note: This tricks `docker build` into using bash instead of
#       the dorked /bin/sh that is part of ubuntu base image.
# Note: This should be done first so all subsequent environment
#       settings are part of this new shell environment.
RUN rm /bin/sh && ln -s /bin/bash /bin/sh

ARG DEBIAN_FRONTEND=noninteractive

# Provide shell a namespace it's running in.
#ARG image_name
#ENV DOCKER_IMAGE_NAME=${image_name}

# Prompts without color should be the edge case.
#ENV TERM=screen-256color

# Break that absurd bell!
#RUN echo "set bell-style none" >> /etc/inputrc

# Update repository list and install baseline dependencies.
RUN apt-get update && apt-get install -y \
    net-tools iputils-ping \
    curl wget \
    python3 \
    vim vim-common \
    rsync \
    git \
    openssh-client telnet \
    && apt-get -y autoclean

# Allow sudoers to sudo without passwords.
# Note: Recommended to use `sudo -E` in dockerfiles.
#RUN apt-get install -y sudo
#RUN sed -i 's/ALL=(ALL:ALL) ALL/ALL=(ALL:ALL) NOPASSWD: ALL/' /etc/sudoers

RUN apt-get update && apt-get install -y \
    build-essential \
    flex bison \
    libssl-dev \
    crossbuild-essential-arm64 \
    libncurses5-dev \
    bc \
    cpio

# User defaults, ideally setup to match existing user's info with `whoami` and `id`.
ARG uid=1000
ARG gid=1000
ARG username=user
ARG groupname=user
ARG groups=user

RUN addgroup --gid ${gid} ${groupname}
RUN adduser --system --disabled-password --uid ${uid} --gid ${gid} ${username}

# Create and change ownership of /opt to user.
WORKDIR /opt/boxbuild
WORKDIR /opt/kbuild
RUN chown -R ${uid}:${gid} /opt

USER ${uid}:${gid}

RUN rsync -rv /etc/skel/ /home/${username}/
RUN echo "alias sudo='sudo -E'" >> /home/${username}/.bashrc

# Clone linux source.
RUN git clone --depth=1 https://github.com/raspberrypi/linux src && mkdir build

# Create folder for init files.
WORKDIR /opt/kbuild/init

# Change to linux src folder.
WORKDIR /opt/kbuild/src

# Set environment variable for cross compilation.
ENV KERNEL=kernel8
ENV ARCH=arm64
ENV CROSS_COMPILE=aarch64-linux-gnu-

# Initialize linux build.
RUN make ARCH=arm64 O=../build CROSS_COMPILE=aarch64-linux-gnu- bcm2711_defconfig

WORKDIR /opt/kbuild/build

# Copy linux build config.

# Perform linux build.
RUN make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- Image modules dtbs

# Busybox Build Stuff
WORKDIR /opt/boxbuild

USER root
RUN DEBIAN_FRONTEND=noninteractive apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y \
    libncurses5-dev bc cpio fakeroot vim vim-common

# Create sysroot
WORKDIR /opt/boxbuild/sysroot
RUN chown -R ${uid}:${gid} /opt/boxbuild/sysroot
USER ${uid}:${gid}

# Clone 1.2.2 of musl
WORKDIR /opt/boxbuild
RUN git clone git://git.musl-libc.org/musl
WORKDIR /opt/boxbuild/musl
RUN git checkout v1.2.2

RUN CROSS_COMPILE=aarch64-linux-gnu- ./configure \
  --enable-wrapper=gcc --target=aarch64-linux-gnu \
  --prefix=/opt/boxbuild/sysroot
RUN CROSS_COMPILE=aarch64-linux-gnu- make install

WORKDIR /opt/kbuild/build
RUN make headers_install ARCH=arm64 INSTALL_HDR_PATH=/opt/boxbuild/sysroot

WORKDIR /opt/boxbuild
RUN git clone https://git.busybox.net/busybox
WORKDIR /opt/boxbuild/busybox
RUN git checkout 1_35_0
COPY --chown=${uid}:${gid} busybox-1.35.config .config
RUN make install

USER root
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y u-boot-tools
USER ${uid}:${gid}

#WORKDIR /opt/boxbuild/rootfs
#COPY --chown=${uid}:${gid} 5.10.52-v8+.tar.gz /opt/boxbuild/5.10.52-v8+.tar.gz
#RUN mkdir -p lib/modules
#WORKDIR /opt/boxbuild/rootfs/lib/modules
#RUN tar -xf /opt/boxbuild/5.10.52-v8+.tar.gz

WORKDIR /opt/boxbuild/rootfs
COPY --chown=${uid}:${gid} init.sh init
RUN chmod +x init
WORKDIR /opt/boxbuild
COPY --chown=${uid}:${gid} build-initramfs.sh build-initramfs.sh
RUN chmod +x build-initramfs.sh && fakeroot ./build-initramfs.sh
RUN mkimage -A arm -O linux -T ramdisk -C none -n "RootFilesystem" -d ./initramfs.cpio.gz ./initramfs.uboot

WORKDIR /opt
