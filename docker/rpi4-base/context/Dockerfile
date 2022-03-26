FROM ubuntu:22.04

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

RUN /usr/bin/dpkg --add-architecture arm64

RUN /usr/bin/sed -i 's/deb h/deb [arch=amd64] h/' /etc/apt/sources.list && \
    /usr/bin/echo "" >> /etc/apt/sources.list && \
    /usr/bin/echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports/ jammy main restricted" >> /etc/apt/sources.list && \
    /usr/bin/echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports/ jammy-updates main restricted" >> /etc/apt/sources.list && \
    /usr/bin/echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports/ jammy universe" >> /etc/apt/sources.list && \
    /usr/bin/echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports/ jammy-updates universe" >> /etc/apt/sources.list && \
    /usr/bin/echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports/ jammy multiverse" >> /etc/apt/sources.list && \
    /usr/bin/echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports/ jammy-updates multiverse" >> /etc/apt/sources.list && \
    /usr/bin/echo "" >> /etc/apt/sources.list

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
    cpio \
    fakeroot \
    u-boot-tools \
    libncurses-dev:arm64

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

# Set environment variable for cross compilation.
ENV KERNEL=kernel8
ENV ARCH=arm64
ENV CROSS_COMPILE=aarch64-linux-gnu-
