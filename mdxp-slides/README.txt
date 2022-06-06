# MDXP Slides for Embedded Systems Analysis

## Overview

These are the slide decks for Embedded Systems Analysis. 

These decks are built with a _variant/fork_ of MDXP. This new variant of MDXP has been modified to allow the developer to manage multiple decks within a single repo. Each deck is output into the `dist` folder with its own subfolder.

## Build & Host

Using the same docker image from the course material (crazychenz/node-16-alpine), build each slide deck and then use the result statically:

- `./yarn.sh mdxp:onepage -d InitialVisualAnalysis`
- `./yarn.sh mdxp:onepage -d BehavioralAnalysis`
- `./yarn.sh mdxp:onepage -d HardwareAnalysis`
- `./yarn.sh mdxp:onepage -d SystemLevelSoftware`
- `./yarn.sh mdxp:onepage -d FirmwareAnalysis`
- `./yarn.sh mdxp:onepage -d Application`

You'll find the output for each deck in a `./dist` folder. Host this folder to get access to the decks from Chrome or Edge.

```sh
python3 -m http.server 4272
```

## Modes

You can open the presentation is multiple tabs and position each tab on different screens to provide a synchronized presenter (Alt-P) and presentation mode (Alt-N).

See more hotkeys within slide deck via (Alt-H).

## Other Resources

[Original MDXP Home](https://0phoff.github.io/MDXP/)
