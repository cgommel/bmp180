/*
 * at24c256_i2c.c
 *
 * This script is a demo for AT24C256 EEPROM access via I2C on Cubieboard
 *
 * Author: soloforce, forum.cubietech.com
 * Date: 2013.11.18
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * PAWEL W. OLSZTA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
 
// Cubieboard IO port PB18 & PB19 are configured to TWI1-SCK & TWI1-SDA (I2C-1)
#define DEFAULT_AT24C256_DEV  "/dev/i2c-1"
 
// ATmega AT24C series EEPROM slave devices are addressed from 0x50~0x57
// As for AT24C256, should be 0x50~0x53
#define DEFAULT_AT24C256_ADDRESS 0x50
 
// AT24C256 has 512 pages, 64 bytes each
#define PAGE_BYTES  64
 
typedef unsigned char uint8;
typedef unsigned short uint16;
 
/*
 * Open the i2c bus device
 */
int init_at24c256_i2c(const char* device)
{
    int fd=open(device, O_RDWR);
    if(fd<0){
        perror("Failed to open I2C device!");
        return -1;
    }
 
    ioctl(fd, I2C_TIMEOUT, 3);    // i2c timeout, 1 for 10ms ; if too small, i2c may lose response
    ioctl(fd, I2C_RETRIES, 3);    // i2c retry limit
 
    return fd;
}
 
/*
 * Page write (or partially page write), data_len should less than PAGE_BYTES
 *
 */
int at24c256_write(int fd, int device_addr, uint16 offset, const char* data, int data_len)
{
    struct i2c_rdwr_ioctl_data at24c256;    // structure pass to i2c driver
    struct i2c_msg i2cmsg;
 
    if(data_len>PAGE_BYTES){
        printf("*** Warning: Page write mode should not exceed %d bytes, otherwise the current page will be rolled over!\n",PAGE_BYTES);
    }
 
    at24c256.nmsgs = 1;    // message count in the structure
    at24c256.msgs = &i2cmsg;
    at24c256.msgs[0].len = 2 + data_len;    // the offset address word is 2 bytes long, plus the buf_len
    at24c256.msgs[0].addr = device_addr;    // i2c slave device address
    at24c256.msgs[0].flags = 0;    // 0 for write; 1 for read
    at24c256.msgs[0].buf = (uint8 *)malloc( at24c256.msgs[0].len );
    if(!at24c256.msgs[0].buf){
        printf("at24c256_write(): Failed to allocate memory for i2c message buffer!\n");
        return -1;
    }
 
    at24c256.msgs[0].buf[0] = (uint8)(offset >> 8);    // MSB first
    at24c256.msgs[0].buf[1] = (uint8)(offset);    // LSB
    strncpy( at24c256.msgs[0].buf+2, data, data_len); // copy user's data to the structure buffer
 
    if(ioctl(fd, I2C_RDWR, (unsigned long)&at24c256)<0 ){    // write ioctl data to the i2c bus
        printf("at24c256_write(): Failed to write data to i2c device!\n");
        free(at24c256.msgs[0].buf);
        return -1;
    }
 
    free(at24c256.msgs[0].buf);
    return 0;
}
 
/*
 * Sequential read from AT24C256, data_len should less than PAGE_BYTES
 */
int at24c256_read(int fd, int device_addr, uint16 offset, char* data, int data_len)
{
    struct i2c_rdwr_ioctl_data at24c256;    // structure pass to i2c driver
    struct i2c_msg i2cmsg[2]; // message count in the structure
    char dummy_write_buf[2];
 
    if(data_len>PAGE_BYTES){
        printf("*** Warning: Sequential read should not exceed %d bytes, otherwise the read data will be rolled over!\n",PAGE_BYTES);
    }
 
    // A dummy write operation should be done according to the AT24C256 i2c protocol
    at24c256.nmsgs = 2;    // message count in the structure
    at24c256.msgs = i2cmsg;
    at24c256.msgs[0].len = 2; // the offset address word is 2 bytes long
    at24c256.msgs[0].addr = device_addr;
    at24c256.msgs[0].flags = 0;    // 0 for write;
    at24c256.msgs[0].buf = dummy_write_buf;
    at24c256.msgs[0].buf[0] = (unsigned char)(offset >> 8);    // write address
    at24c256.msgs[0].buf[1] = (unsigned char)(offset);    // write address
 
    // read operation
    at24c256.msgs[1].len = data_len;
    at24c256.msgs[1].addr = device_addr;
    at24c256.msgs[1].flags = 1;    // 1 for read; 0 for write
    at24c256.msgs[1].buf = data;
 
    if (ioctl(fd, I2C_RDWR, (unsigned long)&at24c256) < 0) {
        printf("at24c256_read(): Failed to read data from EEPROM via i2c!\n");
        return -1;
    }
 
    return 0;
}
 
/*
 * Show help message
 */
void show_usage(char* argv[])
{
    printf("%s <-d device> <-a address> [-o offset] [-r length] [-w data] [-h]\n",argv[0]);
    printf("\t-d device: i2c device file, like /dev/i2c-1 by default\n");
    printf("\t-a address: i2c slave device base address, like 0x50 by default\n");
    printf("\t-o offset: offset from base address, like 0x00, 0x10 ...\n");
    printf("\t-r length: read length bytes from at24c256 eeprom\n");
    printf("\t-w data: write to at24c256 eeprom\n");
    printf("\t-h: show help message\n\n");
}
 
int main(int argc, char* argv[])
{
    char* device=DEFAULT_AT24C256_DEV;
    int slave_address=DEFAULT_AT24C256_ADDRESS;
    int offset=0x00; // default offset is 0
    int length=32; // read 32 bytes by default
    char* data=NULL;
    int rw_mode=1; // 0 for write, 1 for read;
 
    if(argc==1){
        show_usage(argv);
        return 0;
    }
 
    int i;
    while ((i = getopt(argc, argv, "d:a:o:r:w:n:h")) >= 0){
        switch(i){
        case 'd':
            device=optarg;
            break;
        case 'a':
            if (sscanf(optarg, "0x%x", &slave_address) != 1) {
            fprintf(stderr, "Cannot parse '%s' as i2c slave device address, example: '0x50'\n", optarg);
                return -1;
            }
            break;
        case 'o':
            if (sscanf(optarg, "%d", &offset) != 1) {
            fprintf(stderr, "Cannot parse '%s' as address offset, example: '32'\n", optarg);
                return -1;
            }
            break;
        case 'r':
            rw_mode=1;
            if (sscanf(optarg, "%d", &length) != 1) {
                fprintf(stderr, "Cannot parse '%s' as length, example: '17'\n", optarg);
                return -1;
            }
            break;
        case 'w':
            rw_mode=0;
            data=optarg;
            length=strlen(data);
            break;
        case 'h':
            show_usage(argv);
            return 0;
            break;
        }
    }
 
    int fd=init_at24c256_i2c(device);
    if(fd<0) return -1;
 
    printf("i2c device file:%s\nslave_address:0x%X\nR/W offset:0x%X\n",device, slave_address, offset);
 
    if(rw_mode==0){
        // test at24c256 write
        at24c256_write(fd, slave_address, offset, data, length);
        printf("try to write: %s (%d bytes)!\n",data,length);
        usleep(10000); // some delay is necessary for the write operation done
    }else if(rw_mode==1){
        // test at24c256 read
        char *buf=(char*)malloc(length+1);
        memset(buf,0,length+1);
        at24c256_read(fd, slave_address, offset, buf, length);
        usleep(10000); // necessary for the read operation done
 
        printf("read from at24c256 eeprom: %s (%d bytes)\n", buf, strlen(buf));
        free(buf);
    }
 
    close(fd);
}
