#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <math.h>
#include <linux/i2c-dev.h>


int main(void)
{
const char* device="/dev/i2c-1";
int fd=open(device, O_RDWR);
    if(fd<0){
        perror("Failed to open I2C device!");
        return -1;
    }
 ioctl(fd, I2C_TIMEOUT, 3);    // i2c timeout, 1 for 10ms ; if too small, i2c may lose response
    ioctl(fd, I2C_RETRIES, 3);    // i2c retry limit
 

}
