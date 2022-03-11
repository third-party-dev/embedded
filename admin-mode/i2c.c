#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define GPIO_BASE (0xfe200000)
// Pin configuration
#define GPFSEL0 (0/sizeof(uint32_t))
// Set output value
#define GPSET0 (0x1c/sizeof(uint32_t))
// Clear output
#define GPCLR0 (0x28/sizeof(uint32_t))
// Check level
#define GPLEV0 (0x34/sizeof(uint32_t))
// Pullup/Pulldown configuration
#define GPIO_PUP_PDN_CNTRL_REG0 (0xe4/sizeof(uint32_t))

// Broadcom calls I2C the Broadcom Serial Controller (BSC)
#define BSC1_BASE (0xfe804000)
// Easier to read alias.
#define I2C1_BASE (BSC1_BASE)
// Control register
#define I2C_C (0/sizeof(uint32_t))
// Status register
#define I2C_S (0x04/sizeof(uint32_t))
// Packet Length register
#define I2C_DLEN (0x08/sizeof(uint32_t))
// Address register
#define I2C_A (0x0c/sizeof(uint32_t))
// 8-bit wide data fifo register
#define I2C_FIFO (0x10/sizeof(uint32_t))

#define I2C_MODE_BYTE (1)

volatile uint32_t * mmap_gpio()
{
  int gpioctrlfd = open("/dev/mem", O_RDWR | O_SYNC);

  if (gpioctrlfd == -1) {
    printf("error opening /dev/mem.\n");
    exit(0);
  }

  volatile uint32_t *gpioctrl = (volatile uint32_t *)mmap((void *)GPIO_BASE, 0x100, PROT_READ | PROT_WRITE, MAP_SHARED, gpioctrlfd, GPIO_BASE);
  if (gpioctrl == (void *)-1) {
    printf("Failed to mmap.\n");
    exit(0);
  }

  return gpioctrl;
}

void init_gpio(volatile uint32_t *gpioctrl)
{
  // Clear config for GPIO 2 and 3 (pin 3 and 5)
  gpioctrl[GPFSEL0] &= ~0x00000FC0;
  // Set GPIO 2 and 3 to Alt Func 0: SDA1/SDC1 (pin 3 and 5)
  gpioctrl[GPFSEL0] |= 0x00000900;

  // Clear GPIO 2 and 3 resistor config.
  gpioctrl[GPIO_PUP_PDN_CNTRL_REG0] &= ~0x000000F0;
  // Set GPIO 2 and 3 pullup resistor config.
  gpioctrl[GPIO_PUP_PDN_CNTRL_REG0] |= 0x00000050;

  printf("GPFSEL0 (0x%08x) = 0x%08x\n", &gpioctrl[GPFSEL0], gpioctrl[GPFSEL0]);
  printf("PUP_PDN (0x%08x) = 0x%08x\n", &gpioctrl[GPIO_PUP_PDN_CNTRL_REG0], gpioctrl[GPIO_PUP_PDN_CNTRL_REG0]);
  printf("GPLEV0 (0x%08x) = 0x%08x\n", &gpioctrl[GPLEV0], gpioctrl[GPLEV0]);
}

void i2c_enable(volatile uint32_t *i2cc)
{
  i2cc[I2C_C] = 0x00008000;
}

void reset_fifo(volatile uint32_t *i2cc)
{
  // Clear FIFO
  i2cc[I2C_C] |= 0x00000030;
}

uint32_t fifo_empty(volatile uint32_t *i2cc)
{
  return (i2cc[I2C_S] >> 6) & 0x1;
}

uint32_t xfer_done(volatile uint32_t *i2cc)
{
  return (i2cc[I2C_S] >> 1) & 0x1;
}

uint32_t xfer_active(volatile uint32_t *i2cc)
{
  return i2cc[I2C_S] & 0x1;
}

uint32_t xfer_error(volatile uint32_t *i2cc)
{
  return (i2cc[I2C_S] >> 8) & 0x1;
}

uint32_t fifo_len(volatile uint32_t *i2cc)
{
  return i2cc[I2C_DLEN] & 0xFFFF;
}

void reset_done(volatile uint32_t *i2cc)
{
  i2cc[I2C_S] = 1 << 1;
}

void write_start(volatile uint32_t *i2cc)
{
  i2cc[I2C_C] |= 0x00000080;
}

void read_start(volatile uint32_t *i2cc)
{
  i2cc[I2C_C] |= 0x00000081;
}

void xfer_block(volatile uint32_t *i2cc)
{
  while (xfer_done(i2cc) != 1) {;}
}

void xfer_mode(volatile uint32_t *i2cc, uint8_t mode)
{
  i2cc[I2C_DLEN] = mode;
}

void i2cset(volatile uint32_t *i2cc, uint8_t chip_addr, uint8_t reg_addr, uint8_t value)
{
  i2cc[I2C_A] = (uint32_t)chip_addr;
  xfer_mode(i2cc, 2);

  // Write Register Address
  reset_fifo(i2cc);
  i2cc[I2C_FIFO] = (uint32_t)reg_addr;
  i2cc[I2C_FIFO] = (uint32_t)value;
  reset_done(i2cc);
  write_start(i2cc);
  xfer_block(i2cc);
}

uint32_t i2cget(volatile uint32_t *i2cc, uint8_t chip_addr, uint8_t reg_addr)
{
  i2cc[I2C_A] = (uint32_t)chip_addr;
  xfer_mode(i2cc, I2C_MODE_BYTE);

  // Write Register Address
  reset_fifo(i2cc);
  i2cc[I2C_FIFO] = (uint32_t)reg_addr;
  reset_done(i2cc);
  write_start(i2cc);
  xfer_block(i2cc);

  // Read Register Value
  reset_fifo(i2cc);
  reset_done(i2cc);
  read_start(i2cc);
  xfer_block(i2cc);

  return i2cc[I2C_FIFO] & 0xFF;
}

volatile uint32_t *mmap_i2cc()
{
  int i2ccfd = open("/dev/mem", O_RDWR | O_SYNC);

  if (i2ccfd == -1) {
    printf("error opening /dev/mem for i2c controller.\n");
    exit(0);
  }

  volatile uint32_t *i2cc = (volatile uint32_t *)mmap((void *)I2C1_BASE, 0x20, PROT_READ | PROT_WRITE, MAP_SHARED, i2ccfd, I2C1_BASE);
  if (i2cc == (void *)-1) {
    printf("Failed to mmap for I2C1 controller.\n");
    exit(0);
  }

  return i2cc;
}

uint8_t challenge(volatile uint32_t *i2cc, uint8_t chip_addr, uint8_t p1, uint8_t p2)
{
  //printf("Set the nibbles we want to XOR (`XOR(0xA, 0x3)`) in GPIOB\n");
  i2cset(i2cc, chip_addr, 0x13, p1 << 4 | p2);

  //printf("If everything is good, we'll get `0x09` from GPIOA\n");
  return i2cget(i2cc, chip_addr, 0x12);
}

int main(int argc, void **argv)
{
  if (argc < 3) {
    printf("Usage: %s <p1> <p2>\n", argv[0]);
    exit(1);
  }

  volatile uint32_t *gpioctrl = mmap_gpio();
  init_gpio(gpioctrl);

  volatile uint32_t *i2cc = mmap_i2cc();
  i2c_enable(i2cc);

  //printf("Set GPIOB port to all outputs\n");
  i2cset(i2cc, 0x20, 0x01, 0x00);

  errno = 0;
  uint8_t p1 = (uint8_t)strtol(argv[1], NULL, 0); //0x5;
  if (errno) {
    printf("<p1> invalid.\n", argv[1]);
    exit(1);
  }

  errno = 0;
  uint8_t p2 = (uint8_t)strtol(argv[2], NULL, 0); //0xA;
  if (errno) {
    printf("<p1> invalid.\n", argv[1]);
    exit(1);
  }

  uint8_t result = challenge(i2cc, 0x20, p1, p2);
  //! BUG: Reverse result?
  //result = (result & 0x8) >> 3 | (result & 0x4) >> 1 | (result & 0x2) << 1 | (result & 0x1) << 3;
  printf("xor(0x%02x, 0x%02x) = 0x%02x (expected 0x%02x)\n", p1, p2, result, p1 ^ p2);
  if (result == (p1 ^ p2)) {
    printf("Passed!\n");
  } else {
    printf("Failed!\n");
  }

  return 0;
}