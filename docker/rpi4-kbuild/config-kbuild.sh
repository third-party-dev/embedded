#!/bin/sh

if [ ! -e kbuild ]; then mkdir kbuild ; fi
rsync -av context/ kbuild/

docker run -ti --rm \
  -v $(pwd)/kbuild:/opt/kbuild \
  -w /opt/kbuild crazychenz/rpi4-base \
  /opt/kbuild/_config-kbuild.sh

echo "Remember to `cp kbuild/build/.config context/kbuild.config` to save changes."