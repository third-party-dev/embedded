# Multi-Purpose Module

## Backup Appliance

In the backup appliance mode, the module will automatically download and backup data served up by an external TFTP service.

### Configuration

The administrator must configure the appliance's target IP, subnet, and gateway address, and the duration between backups.

## HTTP Status Provider

The module provides HTTP Status responses for various HTTP Status Errors.

### Configuration

The admin is responsible for setting up the listening IP, subnet, gateway.

## Cryptowallet Database

The module provides an interface for saving cryptowallet recovery information.

### Configuration

The admin must configure the listening IP, subnet, gateway.

## Advanced Topics

The module can run in a user mode or an administrator mode. The default is for the device to run in user mode.

### Administration Dongle

To start the device in Admin Mode, an administration dongle must be connected to the device's I2C1 interface at boot time. The system verifies that the dongle is valid by sending 8 bits of random data to its GPIOA port. The dongle is responsible for XORing the 2 x 4bit nibbles together and returning the 4bit result on the GPIOB port. If everything matches, the system will drop into an Administration Prompt instead of booting directly into the kernel.
