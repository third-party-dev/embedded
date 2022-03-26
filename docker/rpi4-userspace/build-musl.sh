#!/bin/sh

if [ ! -e boxbuild ]; then mkdir boxbuild ; fi
rsync -av context/ boxbuild/

docker run -ti --rm \
  -v $(pwd)/boxbuild:/opt/boxbuild \
  -v $(pwd)/../rpi4-kbuild/kbuild:/opt/kbuild \
  -w /opt/boxbuild crazychenz/rpi4-base \
  /opt/boxbuild/_build-musl.sh