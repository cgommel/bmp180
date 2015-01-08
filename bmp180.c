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

typedef struct {
	int16_t AC1; // =   408;
	int16_t AC2; // =   -72;
	int16_t AC3; // =-14383;
	uint16_t AC4; //=32741;
	uint16_t AC5; //=32757;
	uint16_t AC6; //=23153;
	int16_t B1; //=   6190;
	int16_t B2; //=      4;
	int16_t MB; //= -32768;
	int16_t MC; //=  -8711;
	int16_t MD; //=   2868;
}bmp180_calibration;

#define BMP180_I2C_ADDR (0xAA)

int bmp180_read_cal (int fd, bmp180_calibration* cal)
{
    int res=0;
	res|=reg_read_short(fd,BMP180_I2C_ADDR,0xaa,(uint16_t*)&cal->AC1);
	res|=reg_read_short(fd,BMP180_I2C_ADDR,0xac,(uint16_t*)&cal->AC2);
	res|=reg_read_short(fd,BMP180_I2C_ADDR,0xae,(uint16_t*)&cal->AC3);
	res|=reg_read_short(fd,BMP180_I2C_ADDR,0xb0,(uint16_t*)&cal->AC4);
	res|=reg_read_short(fd,BMP180_I2C_ADDR,0xb2,(uint16_t*)&cal->AC5);
	res|=reg_read_short(fd,BMP180_I2C_ADDR,0xb4,(uint16_t*)&cal->AC6);

	res|=reg_read_short(fd,BMP180_I2C_ADDR,0xb6,(uint16_t*)&cal->B1);
	res|=reg_read_short(fd,BMP180_I2C_ADDR,0xb8,(uint16_t*)&cal->B2);

	res|=reg_read_short(fd,BMP180_I2C_ADDR,0xba,(uint16_t*)&cal->MB);
	res|=reg_read_short(fd,BMP180_I2C_ADDR,0xbc,(uint16_t*)&cal->MC);
	res|=reg_read_short(fd,BMP180_I2C_ADDR,0xbe,(uint16_t*)&cal->MD);

	return res;
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
