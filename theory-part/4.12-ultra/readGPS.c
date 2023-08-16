
#include "set_opt.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define INFOSIZE 64

/* 存储gps信息的结构体 */
struct gpsInfo 
{
    /* 时间，纬度，南北半球，经度，东西半球 */
    char* time;    
    char* lat;
    char* lon;
    char* north_south;
    char *east_west;
};


/*
 * gps的信息每一段都是以$开头，\r\n结尾，
 * 我们所需要分解的是如下格式的信息段, 但是读入信息是将每段信息都读，是否要处理由处理函数决定
 * $GPGGA，<1>，<2>，<3>，<4>，<5>，<6>，<7>，<8>，<9>，M，<10>，M，<11>，<12>*hh<CR><LF>
 */
int getRawData(int fd, char *buf, size_t buflen)
{
    char c;
    int cnt = 0;

    while(1)
    {
        if(read(fd, &c, 1) != 1)
        {
            fprintf(stderr, "Error reading\n");
            return -1;
        }

        if(c == '$')
            break;   
    }
    buf[cnt++] = c;

    while(cnt < buflen)
    {
        if(read(fd, &c, 1) != 1)
        {
            fprintf(stderr, "Error reading\n");
            return -1;
        }
        
        if(c == '\n' || c == '\r')
        {
            return 0;
        }
        else
        {
            buf[cnt++] = c;
        }
    }

    if(cnt >= buflen)
    {
        fprintf(stderr, "Error : buf is not enough\n");
        return -1;
    }

    return 0;
}

/* 处理gps信息的函数 */
int parseRawData(char *buf, int buflen, struct gpsInfo *info)
{
    char tmp[32];

#ifdef PRINT
    puts(buf); /* 打印原始信息 */
#endif

    if(buf[0] != '$')
    {
        return 0;
    }
    else if(strncmp(buf + 3, "GGA", 3) != 0)
    {
        return 0;
    }
    else if(strstr(buf, ",,,,,"))
    {
        puts("Inforamtion is lost");
        return 0;
    }
    else
    {
        sscanf(buf,"%[^,},%[^,],%[^,],%[^,],%[^,],%[^,],", tmp, info->time, info->lat, info->north_south, info->lon, info->east_west);
        return 1;
    }
    
}

int main(int argc, char **argv)
{
    int fd;
    char buf[1024];
    struct gpsInfo info = 
    {
        .east_west = calloc(INFOSIZE, sizeof(char)),
        .lat = calloc(INFOSIZE, sizeof(char)),
        .lon = calloc(INFOSIZE, sizeof(char)),
        .north_south = calloc(INFOSIZE, sizeof(char)),
        .time = calloc(INFOSIZE, sizeof(char))
    };

    if(argc != 2)
    {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        return -1;
    }

    /* O_NOCTTY 不让此进程称为打开的终端串口的控制进程 */
    fd = open(argv[1], O_RDWR | O_NOCTTY);
    if(fd < 0)
    {
        fprintf(stderr, "Error opening %s\n", argv[1]);
        return -1;
    }
    if(fcntl(fd, F_SETFL, 0))
    {
        fprintf(stderr, "Error setting 'BLOCK' flags on %s\n", argv[1]);
        return -1;
    }

    set_opt(fd, 9600, 8, 'N', 1);

    while(1)
    {
        getRawData(fd, buf, sizeof(buf));

        if(parseRawData(buf, strlen(buf), &info))
        {
            printf("time : %s\n", info.time);
            printf("lattitude : %s\n", info.lat);
            printf("longitude : %s\n", info.lon);
            printf("East or West : %s\n", info.east_west);
            printf("North or South : %s\n", info.north_south);
        }
        sleep(2); /* 每两秒打印一次 */
    }

    close(fd);
    return 0;
}