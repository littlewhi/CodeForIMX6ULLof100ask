#include "i2cbusses.h"
#include <errno.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <time.h>
#include <stdio.h>

#define AP3216_ADDR 0x1e
#define ENABLE_REG 0x0
#define ENABLE_FLAG 0x3
#define RESET_REG 0x0
#define RESET_FLAG 0x4
#define READ_LIGHT_REG 0xc
#define READ_DISTANCE_REG 0xe

#define WRITE_FLAG 0

static int fd;

static struct i2c_msg *set_msg( struct i2c_msg *msg, int flag, int addr, unsigned char *buf, int len )
{
    msg->flags = flag;
    msg->addr = addr;
    msg->len = len;
    msg->buf = buf;

    return msg;
}

void ap3216_init( void )
{
    char filename[128];
    struct i2c_msg msg;
    unsigned char buf[512];
    struct i2c_rdwr_ioctl_data rdwr;

    struct timespec time;    

    /* 打开‘适配器/总线’ */
    fd = open_i2c_dev( 0, filename, sizeof( filename ), 0 );
    /* 按照地址设置从设备 */
    if( set_slave_addr( fd, AP3216_ADDR, 1 ) )
        return;
    
    time.tv_sec = 0;
    time.tv_nsec = 10000000;    /* 10ms */
    
    /* 复位 */
    buf[0] = RESET_REG;
    buf[1] = RESET_FLAG;
    rdwr.msgs = set_msg( &msg, WRITE_FLAG, AP3216_ADDR, buf, 2 );
    rdwr.nmsgs = 1;
    ioctl( fd, I2C_RDWR, &rdwr );
    nanosleep( &time, NULL );
    
    /* 使能 */
    buf[0] = ENABLE_REG;
    buf[1] = ENABLE_FLAG;
    rdwr.msgs = set_msg( &msg, WRITE_FLAG, AP3216_ADDR, buf, 2 );
    rdwr.nmsgs = 1;
    ioctl( fd, I2C_RDWR, &rdwr );
    nanosleep( &time, NULL );
}

int ap3216_read_light_data( void )
{
    struct i2c_msg msg[2];
    unsigned char wbuf[16], rbuf[16];
    struct i2c_rdwr_ioctl_data rdwr;

    struct timespec time = {
        0, 10000000
    };

    /* 写入要读的寄存器地址 */
    wbuf[0] = READ_LIGHT_REG;
    rdwr.msgs = set_msg( msg, WRITE_FLAG, AP3216_ADDR, wbuf, 1);
    set_msg( msg + 1, I2C_M_RD, AP3216_ADDR, rbuf, 2);
    rdwr.nmsgs = 2;

    ioctl( fd, I2C_RDWR, &rdwr );
    nanosleep( &time, NULL );
    
    return ( (int) rbuf[1] << 8 ) | rbuf[0];
}

int ap3216_read_distance_data( void )
{
    struct i2c_msg msg[2];
    unsigned char wbuf[16], rbuf[16];
    struct i2c_rdwr_ioctl_data rdwr;

    struct timespec time = {
        0, 10000000
    };

    wbuf[0] = READ_DISTANCE_REG;
    rdwr.msgs = set_msg( msg, WRITE_FLAG, AP3216_ADDR, wbuf, 1 );
    set_msg( msg + 1, I2C_M_RD, AP3216_ADDR, rbuf, 2 );
    rdwr.nmsgs = 2;
    
    ioctl( fd, I2C_RDWR, &rdwr );

    return ( ((int) rbuf[1] & 0x3f) << 4 ) | ( rbuf[0] & 0xf );
}

int main( void )
{
    ap3216_init();
    while(1)
    {
        puts( "continue?" );
        if( getchar() == EOF )
            break;
        printf( "light strength: %d\n", ap3216_read_light_data() );
        printf( " distance: %d\n", ap3216_read_distance_data() );
    }

    close( fd );

    return 0;
}