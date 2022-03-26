#include <stdio.h>
#include <stdint.h>
#include <sched.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <time.h>
#include <string.h>
#include <errno.h>

/*

Add to config.txt:

  gpio=5,17,22,23=pu

Run on command line:

  echo 5 > /sys/class/gpio/export
  echo "rising" > /sys/class/gpio/gpio5/edge
  echo 17 > /sys/class/gpio/export
  echo "rising" > /sys/class/gpio/gpio17/edge
  echo 22 > /sys/class/gpio/export
  echo "rising" > /sys/class/gpio/gpio22/edge
  echo 23 > /sys/class/gpio/export
  echo "rising" > /sys/class/gpio/gpio23/edge

*/

#define POLL_GPIO POLLPRI | POLLERR 

void open_pin(struct pollfd *pin, const char *pin_path)
{
    int fd = open(pin_path, O_RDONLY);
    if (fd < 0) {
        printf("Error opening pin: %s\n", pin_path);
        exit(1);
    }

    pin->fd = fd;
    pin->events = POLL_GPIO;
    pin->revents = 0;
}

void poll_pins(struct pollfd *pins, nfds_t pins_cnt)
{
    int poll_ret = poll(pins, pins_cnt, 10*1000);

    if (!poll_ret) {
        // Timeout
        return;
    }
        
    if (poll_ret == -1) {
        perror("poll");
        exit(EXIT_FAILURE);
    }

    for (nfds_t i = 0; i < pins_cnt; ++i) {
        if (pins[i].revents & POLL_GPIO) {
            uint8_t value;
            lseek(pins[i].fd, 0, SEEK_SET);
            read(pins[i].fd, (char *)&value, 1);
            printf("GPIO %d %02x\n", i, value);
            pins[i].revents = 0;
            continue;
        }
    }
}

int main(int argc, char *argv[]){
    const nfds_t pins_cnt = 4;
    struct pollfd pins[pins_cnt];

    open_pin(&pins[0], "/sys/class/gpio/gpio5/value");
    open_pin(&pins[1], "/sys/class/gpio/gpio17/value");
    open_pin(&pins[2], "/sys/class/gpio/gpio22/value");
    open_pin(&pins[3], "/sys/class/gpio/gpio23/value");

    while(1) {
        poll_pins(pins, pins_cnt);
	}

    for (int i = 0; i < pins_cnt; ++i) {
        close(pins[i].fd); //close value file
    }

    return EXIT_SUCCESS;
}


