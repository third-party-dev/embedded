#!/bin/sh

docker run --network host -ti --rm -v $(pwd):/workspace -w /workspace -u 1000 crazychenz/node-16-alpine yarn "$@"
