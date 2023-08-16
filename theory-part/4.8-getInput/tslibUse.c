
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h>
#include <linux/input.h>

#include <sys/ioctl.h>

#include <tslib.h>


double dist(struct ts_sample_mt *, struct ts_sample_mt *);


/* usage : name <devname> */
int main(int argc, char *argv[])
{
	struct tsdev *tsp;
	struct ts_sample_mt **samp = NULL;
	struct ts_sample_mt **oldsamp = NULL;
	int maxSlots;
	int nr = 1; /* 每回读取几次数据 */
	int i;
	int points[2];
	struct input_absinfo slot;
	
	tsp = ts_setup(NULL, 0);
	if(tsp == NULL)
	{
		fprintf(stderr, "Cannot find dev\n");
		return -1;
	}
	if(ioctl(ts_fd(tsp), EVIOCGABS(ABS_MT_SLOT), &slot) < 0)
	{
		fprintf(stderr, "Cannot get the number of max slots\n");
		return -1;
	}
	/* 数据存储位置初始化 */
	maxSlots = slot.maximum + 1 - slot.minimum;
	//printf("%d\n", maxSlots);
	samp = malloc(sizeof(int*) * nr);
	oldsamp = malloc(sizeof(int*) * nr);
	
	for(i = 0; i < nr; ++i)
	{
		int j;
		samp[i] = malloc(sizeof(struct ts_sample_mt) * maxSlots);
		memset(samp[i], 0, sizeof(struct ts_sample_mt) * maxSlots);
		oldsamp[i] = malloc(sizeof(struct ts_sample_mt) * maxSlots);
		//printf("0\n");
		//fflush(stdout);
		for(j = 0; j < maxSlots; ++j)
		{
			//printf("j = %d\n", j);
			//fflush(stdout);
			samp[i][j].valid = 0;/* 无效 */
			oldsamp[i][j].valid = 0;
		}
	}
	
	while(1)
	{
	
		ts_read_mt(tsp, samp, maxSlots, nr);
		
		for(i = 0; i < nr; ++i)
		{
			int j, sum;;
			memcpy(oldsamp[i], samp[i], sizeof(struct ts_sample_mt) * maxSlots);
			sum = 0;
			for(j = 0; j < maxSlots; ++j)
			{
				if((oldsamp[i][j].valid & TSLIB_MT_VALID) && oldsamp[i][j].tracking_id != -1 && sum++ < 2)
				{
					points[sum - 1] = j;
				}
				if(sum > 2)
					break;
			}
			if(sum == 2)
			{
				printf("distance : %6.4lf\n", dist(&oldsamp[i][points[0]], &oldsamp[i][points[1]]));
			}	
		}
	
	}
	
	return 0;
}

double dist(struct ts_sample_mt * point1, struct ts_sample_mt *point2)
{
	return sqrt(pow(point1->x - point2->x, 2) + pow(point1->y - point2->y, 2));
}

