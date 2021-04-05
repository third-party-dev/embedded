## Setting Up OpenOCD in Docker from Windows Host

1. Install Virtual Box
2. Install Virtual Box Extension Pack
3. Install Ubuntu >= 18
4. Install docker
5. Add user to docker group (usermod/newgrp)
6. Build image with sudo configured and xpack-openocd-0.11.0-1 installed https://github.com/xpack-dev-tools/openocd-xpack/releases/tag/v0.11.0-1/
7. Run with --network host --privileged -v /dev/bus/usb:/dev/bus/usb
8. sudo openocd (optionally have the user have plugdev access or access to /dev/bus/usb?)

- Need to install Zadig followed by installing WinUSB driver for each interface of device.
https://zadig.akeo.ie/

References:
https://forums.virtualbox.org/viewtopic.php?f=35&t=82639
https://www.virtualbox.org/wiki/Downloads

## Setup FT2232H

### Pin Out Resources

https://pinout.xyz/pinout/jtag
https://ftdichip.com/wp-content/uploads/2020/07/DS_FT2232H_Mini_Module.pdf

### JTAG Pins

```
RPi4 Pin 9 - GND           to FT2232H Pin CN2-6 - GND
RPi4 Pin 22 - GPIO25/TCK   to FT2232H Pin CN2-7 - AD0/TCK
RPi4 Pin 37 - GPIO26/TDI   to FT2232H Pin CN2-10 - AD1/TDI
RPi4 Pin 18 - GPIO24/TDO   to FT2232H Pin CN2-9 - AD2/TDO
RPi4 Pin 13 - GPIO27/TMS   to FT2232H Pin CN2-12 - AD3/TMS
```

**Untested**
```
RPi4 Pin 15 - GPIO22/TRST
RPi4 Pin 16 - GPIO23/RTCK
```

### UART Pins (Optional)

```
RPi4 Pin  8 - GPIO14/TX  to FT2232H Pin CN3-25 - BD0/RX
RPi4 Pin 10 - GPIO15/RX  to FT2232H Pin CN3-26 - BD1/TX
```

### USB Bus Powered Jumps

```
CN3-1 to CN3-3
CN2-3 to CN2-11 *OR* CN2-5 to CN2-21
```

## Setup OpenOCD

http://openocd.org/doc-release/pdf/openocd.pdf

The configuration:

```
# Interface: FT2232H Configuration
adapter driver ftdi
adapter speed 1000
ftdi_vid_pid 0x0403 0x6010
ftdi_channel 0
transport select jtag
ftdi_layout_init 0x0000 0x000b

# Board: Raspberry Pi 4 Configuration
# Nothing to configure.

# Target: BCM2711 Configuration

set _CHIPNAME bcm2711
set _DAP_TAPID 0x4ba00477

transport select jtag
reset_config trst_and_srst

telnet_port 4444

jtag newtap auto0 tap -irlen 4 -expected-id $_DAP_TAPID

dap create auto0.dap -chain-position auto0.tap

set CTIBASE {0x80420000 0x80520000 0x80620000 0x80720000}
set DBGBASE {0x80410000 0x80510000 0x80610000 0x80710000}

set _cores 4

set _TARGETNAME $_CHIPNAME.a72
set _CTINAME $_CHIPNAME.cti
set _smp_command ""

for {set _core 0} {$_core < $_cores} { incr _core} {
    cti create $_CTINAME.$_core -dap auto0.dap -ap-num 0 -baseaddr [lindex $CTIBASE $_core]

    set _command "target create ${_TARGETNAME}.$_core aarch64 \
                    -dap auto0.dap  -dbgbase [lindex $DBGBASE $_core] \
                    -coreid $_core -cti $_CTINAME.$_core"
    if {$_core != 0} {
        set _smp_command "$_smp_command $_TARGETNAME.$_core"
    } else {
        set _smp_command "target smp $_TARGETNAME.$_core"
    }

    eval $_command
}

eval $_smp_command
targets $_TARGETNAME.0
```

OpenOCD error when interface not setup correctly:

```
xPack OpenOCD, x86_64 Open On-Chip Debugger 0.11.0-00155-ge392e485e (2021-03-15-16:43)
Licensed under GNU GPL v2
For bug reports, read
        http://openocd.org/doc/doxygen/bugs.html
Warn : Transport "jtag" was already selected
Info : Listening on port 6666 for tcl connections
Info : Listening on port 4444 for telnet connections
Info : clock speed 1000 kHz
Error: JTAG scan chain interrogation failed: all ones
Error: Check JTAG interface, timings, target power, etc.
Error: Trying to use configured scan chain anyway...
Error: auto0.tap: IR capture error; saw 0x0f not 0x01
Warn : Bypassing JTAG setup events due to errors
Error: Invalid ACK (7) in DAP response
Error: JTAG-DP STICKY ERROR
```

Notice the **`Error: JTAG scan chain interrogation failed: all ones`**

OpenOCD when interface setup without board or target configuration:

```
Open On-Chip Debugger 0.10.0
Licensed under GNU GPL v2
For bug reports, read
        http://openocd.org/doc/doxygen/bugs.html
adapter speed: 1000 kHz
Warn : Transport "jtag" was already selected
trst_and_srst separate srst_gates_jtag trst_push_pull srst_open_drain connect_deassert_srst
Info : clock speed 1000 kHz
Info : JTAG tap: auto0.tap tap/device found: 0x4ba00477 (mfg: 0x23b (ARM Ltd.), part: 0xba00, ver: 0x4)
Warn : gdb services need one or more targets defined
```

OpenOCD Completely Setup:

```
xPack OpenOCD, x86_64 Open On-Chip Debugger 0.11.0-00155-ge392e485e (2021-03-15-16:43)
Licensed under GNU GPL v2
For bug reports, read
        http://openocd.org/doc/doxygen/bugs.html
Warn : Transport "jtag" was already selected
Info : Listening on port 6666 for tcl connections
Info : Listening on port 4444 for telnet connections
Info : clock speed 1000 kHz
Info : JTAG tap: auto0.tap tap/device found: 0x4ba00477 (mfg: 0x23b (ARM Ltd), part: 0xba00, ver: 0x4)
Info : bcm2711.a72.0: hardware has 6 breakpoints, 4 watchpoints
Info : bcm2711.a72.1: hardware has 6 breakpoints, 4 watchpoints
Info : bcm2711.a72.2: hardware has 6 breakpoints, 4 watchpoints
Info : bcm2711.a72.3: hardware has 6 breakpoints, 4 watchpoints
Info : starting gdb server for bcm2711.a72.0 on 3333
Info : Listening on port 3333 for gdb connections
```

When using Ubuntu provided OpenOCD, you may see: apt-get install openocd (v.10) will cause "dap" invalid command error.
https://stackoverflow.com/questions/53714503/openocd-error-invalid-command-name-dap-cant-connect-blue-pill-via-st-link


`ftdi_layout_init 0x0000 0x000b` breaks down as:

The first uint16_t is a mask to indicate which of the 16 GPIOs are data lines.
The second uint16_t is a mask to indicate which of the 16 GPIOs are outputs.

```
1 == openocd output / jtag input
0 == openocd input / jtag output
Pin   AD7 AD6 AD5 AD4 AD3 AD2 AD1 AD0
       0   0   0   0   1   0   1   1  => 0bh
```

At the moment, we only set the 4 least significant bits because we only need 4 lines.

## Minicom Setup

```
+-----------------------------------------------------------------------+
| A -    Serial Device      : /dev/ttyUSB1                              |
| B - Lockfile Location     : /var/lock                                 |
| C -   Callin Program      :                                           |
| D -  Callout Program      :                                           |
| E -    Bps/Par/Bits       : 115200 8N1                                |
| F - Hardware Flow Control : No                                        |
| G - Software Flow Control : No                                        |
|                                                                       |
|    Change which setting?                                              |
+-----------------------------------------------------------------------+
```

## Raspberry Pi 4 Setup

Add the following to `config.txt` to enable JTAG:

```
gpio=22-27=np
enable_jtag_gpio=1
```

Add the following to `config.txt` to enabled mini-UART:

```
enable_uart=1
```

# Running the things.


```
sudo minicom usb1
```

```
./openocd_env.sh sudo xpack-openocd-0.11.0-1/bin/openocd -f ft2232h-module.cfg
```

```
./openocd_env.sh $'gdb-multiarch -ex "target ext 127.0.0.1:3333"'
```

## Tips

- Ctrl-C in GDB to break RPi4.
- `c` in GDB to continue from break.
- Ctrl-C in OpenOCD to stop debugging.

# Other Links

https://www.allaboutcircuits.com/technical-articles/getting-started-with-openocd-using-ft2232h-adapter-for-swd-debugging/
https://techwithdave.davevw.com/2013/07/openocd-ft2232h-based-jtag-adapters.html
https://techwithdave.davevw.com/2013/07/getting-started-with-openocd.html
https://learn.adafruit.com/assets/88382
