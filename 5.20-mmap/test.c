#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define BUFSIZE 1024

int main( void )
{
    int fd;
    char *buf, gotStr[BUFSIZE];
    const char * str = "I am Yang HongBin";
    int slen = strlen(str);

    fd = open("/dev/yhb_mem", O_RDWR);
    if( fd < 0 )
    {
        fprintf( stderr, "Error opening /dev/yhb_mem\n" );
        return -1;
    }

    buf = (char *) mmap( NULL, BUFSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
    if( buf == MAP_FAILED )
    {
        printf( " failed to mmap,\nerror code : %d,\nerror message : %s\n",
         errno, strerror( errno ) );
        return -1;
    }

    strcpy( buf,  str);

    if( read( fd, gotStr, slen ) != slen )
    {
        fprintf( stderr, "read error: %s\n", strerror( errno ) );
        return -1;
    }

    if( strncmp( gotStr, str, slen ) == 0 )
    {
        printf( "Comparison is right.\n" );
    }
    else
    {
        printf( "Comparison is wrong.\n" );
    }

    return 0;
}