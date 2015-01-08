#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <math.h>
#include <linux/i2c-dev.h>
#include <stdint.h>

int reg_write_byte(int handle, uint8_t devaddr, uint8_t regaddr,
		uint8_t content) {
	struct i2c_rdwr_ioctl_data iocall;    // structure pass to i2c driver
	struct i2c_msg message;
	uint8_t buffer[2];

	buffer[0] = regaddr;
	buffer[1] = content;

	iocall.nmsgs = 1;
	iocall.msgs = &message;
	message.addr = devaddr;
	message.flags = 0; //write
	message.buf = (char*) buffer;
	message.len = sizeof(buffer);
	if (ioctl(handle, I2C_RDWR, (unsigned long) &iocall) < 0) {
		printf("error during reg_write_byte()\n");
		return -1;
	}
	return 0;
}

int reg_read_byte(int handle, uint8_t devaddr, uint8_t regaddr,
		uint8_t* content) {
	struct i2c_rdwr_ioctl_data iocall;    // structure pass to i2c driver
	struct i2c_msg messages[2];

	iocall.nmsgs = 2;
	iocall.msgs = messages;

	messages[0].addr = devaddr;
	messages[0].flags = 0; //write
	messages[0].buf = (char*) &regaddr;
	messages[0].len = 1;

	messages[1].addr = devaddr;
	messages[1].flags = I2C_M_RD; //READ
	messages[1].buf = (char*) content;
	messages[1].len = 1;

	if (ioctl(handle, I2C_RDWR, (unsigned long) &iocall) < 0) {
		printf("error during reg_write_byte()\n");
		return -1;
	}
	return 0;
}

int reg_read_short(int handle, uint8_t devaddr, uint8_t regaddr,
		uint16_t* content) {
	struct i2c_rdwr_ioctl_data iocall;    // structure pass to i2c driver
	struct i2c_msg messages[2];
	uint8_t buffer[2];

	iocall.nmsgs = 2;
	iocall.msgs = messages;

	messages[0].addr = devaddr;
	messages[0].flags = 0; //write
	messages[0].buf = (char*) &regaddr;
	messages[0].len = 1;

	messages[1].addr = devaddr;
	messages[1].flags = I2C_M_RD; //READ
	messages[1].buf = (char*) buffer;
	messages[1].len = 2;

	if (ioctl(handle, I2C_RDWR, (unsigned long) &iocall) < 0) {
		printf("error during reg_write_byte()\n");
		return -1;
	}

	*content = (uint16_t) (buffer[0] << 8) | buffer[1];
	return 0;
}

int main(void) {
	const char* device = "/dev/i2c-1";
	int fd = open(device, O_RDWR);
	if (fd < 0) {
		perror("Failed to open device");
		return -1;
	}

	ioctl(fd, I2C_TIMEOUT, 3); // i2c timeout, 1 for 10ms ; if too small, i2c may lose response
	ioctl(fd, I2C_RETRIES, 3);    // i2c retry limit


	return (0);
}
