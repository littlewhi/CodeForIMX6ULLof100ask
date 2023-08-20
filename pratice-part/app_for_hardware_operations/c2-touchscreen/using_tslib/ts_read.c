#include <tslib.h>
#include <stdio.h>

int main( int argc, char **argv )
{
    struct tsdev *ts;

    if( argc != 2 )
    {
        printf( "Usage: %s <devname>\n", argv[0] );
        return -1;
    }

    ts = ts_setup( argv[1], 0 );
    while(1)
    {
		struct ts_sample samp;
		int ret;

		ret = ts_read_raw(ts, &samp, 1);

		if (ret < 0)
        {
			perror("ts_read_raw");
			ts_close(ts);
			return -1;
		}

		if (ret != 1)
			continue;

		printf("%ld.%06ld: %6d %6d %6d\n", samp.tv.tv_sec, samp.tv.tv_usec, samp.x, samp.y, samp.pressure);

	}


    return 0;
}