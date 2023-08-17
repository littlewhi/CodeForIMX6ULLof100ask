#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

static int fd;

void noblock_read( int fd )
{
    int flags;
    struct input_event ie; 
    
    /* 设置无阻塞模式 */
    flags = fcntl( fd, F_GETFL );
    fcntl( fd, F_SETFL, flags | O_NONBLOCK );

    /* 读取数据 */
    while(1)
    {
        if( read( fd, &ie, sizeof(ie) ) != sizeof(ie) )
        {
            fprintf( stderr, "Error reading\n" );
            sleep( 1 );
        }
        else
        {
            printf( "type : %u, code : %u, value : %d\n", ie.type, ie.code, ie.value );
        }
    }
}

void block_read( int fd )
{
    struct input_event ie;

    while(1)
    {
        if( read( fd, &ie, sizeof(ie) ) == sizeof(ie) ) 
        {
            printf( "type : %u, code : %u, value : %d\n", ie.type, ie.code, ie.value );
        }
        else
        {
            fprintf( stderr, "Error reading\n" );
        }
    }
}

void poll_read( int fd )
{
    struct input_event ie;
    struct pollfd pfds[1];

    /* 设置poll数据结构 */
    pfds[0].fd = fd;
    pfds[0].events = POLLIN;

    while(1)
    {
        int ret =  poll( pfds, 1, 5000 );
        if( ret == 1 && pfds[0].revents == POLLIN )       /* 有数据可读 */
        {
            if( read(fd, &ie, sizeof(ie)) == sizeof(ie) ) 
            {
                printf( "type : %u, code : %u, value : %d\n", ie.type, ie.code, ie.value );
            }  
            else 
            {
                fprintf(stderr, "Error reading\n");
            }
        }
        else if( ret == 0 )                             /* 超时 */
        {
            printf( "poll timeout\n" );
        }
        else                                            /* error */
        {
            printf( "Error poll\n" );\
            puts( strerror( errno ) );
        }
    }

}

static void my_sig_handler( int sig )
{
    struct input_event ie;

    if( read(fd, &ie, sizeof(ie)) == sizeof(ie) ) 
    {
        printf( "type : %u, code : %u, value : %d\n", ie.type, ie.code, ie.value );
    }  
    else 
    {
        fprintf(stderr, "Error reading\n");
    }
}

void fasync_read( int fd )
{
    int flags;

    signal( SIGIO, my_sig_handler );

    flags = fcntl( fd, F_GETFL );
    fcntl( fd, F_SETFL, flags | FASYNC );
    fcntl( fd, F_SETOWN,  getpid() );

    while(1)
    {
        puts( "I am working\n" );
        sleep( 2 );
    }
}

int main( int argc, char **argv )
{
    if( argc != 3 )
    {
        printf( "Usage: %s <devname> <switch>\n", argv[0] );
        printf( "Example: %s /dev/input/event1 0\n", argv[0] );
        printf( "switch : 0 - nonblock\n" );
        printf( "\t1 - block\n" );
        printf( "\t2 - poll\n" );
        printf( "\t3 - fasync\n" );
        return -1;
    }

    fd = open( argv[1], O_RDONLY );
    if( fd < 0 )
    {
        fprintf( stderr, "fail to open %s\n", argv[1] );
        return -1;
    }
    
    switch( argv[2][0] )
    {
        case '0':
            noblock_read( fd );
            break;
        case '1':
            block_read( fd );
            break;
        case '2':
            poll_read( fd );
            break;
        case '3':
            fasync_read( fd );
            break;
        default:
            printf( "Wrong switch argument\n" );
            break;   
    }

    close( fd );
    return 0;

}