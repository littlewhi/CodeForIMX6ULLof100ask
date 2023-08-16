
#include "i2c/smbus.h"
#include "i2cbusses.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define SBUS 0
#define DEVICE_ADDR 0x50
    
int i2c_ctl;

int myI2C_read(int n, char *buf)
{
    int i;
    char mem = 0;
    for(i = 0; i < n; ++i)
    {
        /* block读以i2c形式最多是32个，smbus是5个 */
        buf[i] = i2c_smbus_read_byte_data(i2c_ctl, mem++);
        if(buf[i] < 0)
            return -1;
    }

    return 0;
}

int myI2C_write(char *buf, int len)
{
    int i;
    struct timespec req;
    char mem = 0;
    
    for(i = 0; i < len; ++i)
    {
        if(i2c_smbus_write_byte_data(i2c_ctl, mem++, buf[i]))
            return -1;
        
        /* 每一次得写都需要时间 */
        req.tv_sec = 0;
        req.tv_nsec = 20000000;
        nanosleep(&req, NULL);
    }
    
    return 0;
}

/* 
 * i2c_test r <number>
 * i2c_test w <string>
 */
int main(int argc, char **argv)
{

    char filename[32]; 

    if(argc != 3)
    {
myerr:  printf("Usage: %s r <number> or i2c_test w <string>\n", argv[0]);
        return 1;
    }

    /* int open_i2c_dev(int i2cbus, char *filename, size_t size, int quiet) 
     * 从函数源码得
     * i2cbus ——> i2c总线号
     * filename -> 此函数会将适配器或总线的文件名拷贝到此数组
     * size -> filename数组的大小
     * quiet -> 是否保持安静（打印错误信息的形式）
     */
    i2c_ctl = open_i2c_dev(0, filename, sizeof(filename), 0); /* 打开i2c控制层 */

    if(i2c_ctl < 0)
        return -1;
        
    /* int set_slave_addr(int file, int address, int force)
     * file -> 就是open得返回值
     * address -> i2c接入得具体设备的地址，本实验是EEPROM -> address = 0x50
     * force -> force非零时不管设备是否已有驱动，直接访问
     */ 
    if(set_slave_addr(i2c_ctl, DEVICE_ADDR, 1)) /* 将控制层与具体设备连接起来 */
        return -1;
    
    if(strcmp(argv[1], "r") == 0)
    {
        int n;
        char *buf;

        n = atoi(argv[2]);
        buf = malloc((n + 1) * sizeof(char));
        memset(buf, 0, (n + 1) * sizeof(char));

        if(myI2C_read(n, buf))
        {
            fprintf(stderr, "ERROR : failed to read\n");
            free(buf);
            close(i2c_ctl);
            return -1;
        }
        printf("\nREAD : %s\n", buf);
        free(buf);
    }
    else if(strcmp(argv[1], "w") == 0)
    {
        if(myI2C_write(argv[2], strlen(argv[2])))
        {
            fprintf(stderr, "ERROR : failed to write\n");
            close(i2c_ctl);
            return -1;
        }
        printf("\nWRITE : %s\n", argv[2]);
    }
    else
    {
        close(i2c_ctl);
        goto myerr;
    }

    close(i2c_ctl);
    return 0;
}
