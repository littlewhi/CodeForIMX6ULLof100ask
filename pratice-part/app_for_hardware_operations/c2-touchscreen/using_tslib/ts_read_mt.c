#include <tslib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <errno.h>
#include <stdlib.h>

int main( int argc, char **argv )
{
    int i, j;

    int nr = 1;
    struct tsdev *ts;
    struct input_absinfo slot;
    int max_slots;
    struct ts_sample_mt **samples_mt = NULL;

    if( argc != 2 )
    {
        printf( "Usage: %s <devname>\n", argv[0] );
        return -1;
    }

    ts = ts_setup( argv[1], 0 );

    /*
     * 获取绝对输入类型的信息，
     * 计算最大触点数量
     */
    if( ioctl( ts_fd(ts), EVIOCGABS(ABS_MT_SLOT), &slot ) < 0 ) 
    {
		perror("ioctl EVIOGABS");
		ts_close(ts);
		return errno;
	}
    max_slots = slot.maximum - slot.minimum + 1;

    /* 
     * 设置存储输入信息的数据结构 
     * nr是一个触点读取的次数
     * (便宜写代码，没有处理错误) 
     */
    samples_mt = (struct ts_sample_mt **) malloc( nr * sizeof( struct ts_sample_mt * ) );
    for( i = 0; i < nr; i++ )
        samples_mt[i] = (struct ts_sample_mt *) calloc( max_slots, sizeof( struct ts_sample_mt ) );
    

    while(1)
    {
        int ret;
        /* 正常返回值是读取了几次 */
        if( (ret = ts_read_mt( ts, samples_mt, max_slots, nr ) ) < 0 )
        {
            printf( "ts_read_raw_mt failed reading samples\n" );
            continue;
        }
        for (j = 0; j < ret; j++) 
        {
			for (i = 0; i < max_slots; i++) 
            {
				if (!(samples_mt[j][i].valid & TSLIB_MT_VALID))
					continue;

				printf("sample %d, time = %ld.%06ld - (slot %d) x = %6d, y = %6d pressure = %6d\n",
				       j,
				       samples_mt[j][i].tv.tv_sec,
				       samples_mt[j][i].tv.tv_usec,
				       samples_mt[j][i].slot,
				       samples_mt[j][i].x,
				       samples_mt[j][i].y,
				       samples_mt[j][i].pressure);
			}
		}
	}

    return 0;
}