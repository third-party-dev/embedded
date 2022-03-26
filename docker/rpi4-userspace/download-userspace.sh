#!/bin/sh

if [ ! -e boxbuild ]; then mkdir boxbuild ; fi
rsync -av context/ boxbuild/

docker run -ti --rm \
  -v $(pwd)/boxbuild:/opt/boxbuild \
  -w /opt/boxbuild crazychenz/rpi4-base \
  /opt/boxbuild/_download-userspace.sh