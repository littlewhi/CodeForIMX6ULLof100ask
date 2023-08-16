
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/* usage : name devname [NOBLOCK]*/
int main(int argc, char* argv[])
{
	int fd;
	int error;
	int len;
	int i;
	struct input_event event;
	struct input_id id;
	unsigned int evbit[2];
	char *ev_names[] = {
		"EV_SYN ",
		"EV_KEY ",
		"EV_REL ",
		"EV_ABS ",
		"EV_MSC ",
		"EV_SW	",
		"NULL ",
		"NULL ",
		"NULL ",
		"NULL ",
		"NULL ",
		"NULL ",
		"NULL ",
		"NULL ",
		"NULL ",
		"NULL ",
		"NULL ",
		"EV_LED ",
		"EV_SND ",
		"NULL ",
		"EV_REP ",
		"EV_FF	",
		"EV_PWR ",
		};
	

	if(argc < 2)
	{
		fprintf(stderr, "usage : %s <device name> [NOBLOCK]\n", argv[0]);
		return -1;
	}
	
	if(argc == 3 && !strcmp("NOBLOCK", argv[2]))
		fd = open(argv[1], O_RDWR | O_NONBLOCK);
	else
		fd = open(argv[1], O_RDWR);
		
	if(fd < 0)
	{
		fprintf(stderr, "Cannot open %s\n", argv[1]);
		return -1;
	}

	error = ioctl(fd, EVIOCGID, &id);
	if(error == 0)
	{
		printf("bustype = 0x%x\nvendor = 0x%x\nproduct = 0x%x\nversion = 0x%x\n", id.bustype, id.product, id.vendor, id.version);
	}

	len = ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), &evbit);

	if(len > 0 && len <= sizeof(evbit))
	{
		for(i = 0; i < len; ++i)
		{
			int bit = 0;
			for(; bit < 8; ++bit)
			{
				if(((unsigned char*)evbit)[i] & (1 << bit))
				{
					printf("%s\n", ev_names[i*8 + bit]);
				}
			}
		}
		fflush(stdout);
	}



	/* 读信息 */
	while(1)
	{
		len = read(fd, &event, sizeof(event));
		if(len > 0)
		{
			printf("type = %x\ncode = %x\nvalue = %x\n", event.type, event.code, event.value);
		}
		else if(!len)
		{
			printf("no read\n");
		}
		else
		{
			printf("error : %s\n", strerror(errno));
		}
	}
		
	return 0;
}

