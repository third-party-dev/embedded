#!/bin/sh

if [ ! -e uboot ]; then mkdir uboot ; fi
rsync -av context/ uboot/

docker run -ti --rm \
  -v $(pwd)/uboot:/opt/uboot \
  -w /opt/uboot crazychenz/rpi4-base \
  /opt/uboot/_build-uboot.sh