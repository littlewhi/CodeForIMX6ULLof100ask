#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

/**/

static struct fb_var_screeninfo var;
static int frameSize;

void lcd_put_pixel(void*, int, int, int, int, unsigned int);

int main(int argc, char* *argv)
{

    int i, j;
    int fd_fb = open("/dev/fb0", O_RDWR);//打开显示器
    int lineWidth;
    void *fb_base;

    if(fd_fb < 0)
    {
        fprintf(stderr, "Can't open framebuffer.\n");
        return -1;
    }

    if(ioctl(fd_fb, FBIOGET_VSCREENINFO, &var))
    {
        fprintf(stderr, "Cannot get variable information of framebuffer.\n");
        return -1;
    }

    lineWidth = var.xres * var.bits_per_pixel / 8;
    frameSize = var.xres * var.yres * var.bits_per_pixel / 8;
    fb_base = mmap(NULL, frameSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);
    if((unsigned char*)fb_base == (unsigned char*)-1)
    {
        fprintf(stderr, "Can't map framebuffer in memory.\n");
        return -1;
    }
 
    memset(fb_base, -1, frameSize);

    for(i = 0; i < 10; ++i)
    {
        for(j = 0; j < 10; ++j)
        { 
            printf("i = %d, j = %d\n", i, j);
            fflush(stdout);
            lcd_put_pixel(fb_base, lineWidth, var.bits_per_pixel / 8, var.xres / 2 + i, var.yres / 2 + j, 0x00ff0000);
        }
    }

    munmap(fb_base, frameSize);
    close(fd_fb);

    return 0;

}

void lcd_put_pixel(void *base, int lineWidth, int unit, int x, int y, unsigned int color)
{
    unsigned char *pen_8 = ((char*)base) + y * lineWidth  + x * unit;
    unsigned short *pen_16 = (unsigned short*)pen_8;
    unsigned int *pen_32 = (unsigned int*)pen_8;
    int green, red, blue; 
    switch(unit)
    {
        case 1:
            *pen_8 = (char)color;
            break;
        case 2:
            red = (color >> 16) & 0xf8;
            green = (color >> 8) & 0xfc;
            blue = (color) & 0xf8;
            *pen_16 = (unsigned short)((red << 11) | (green << 5) | (blue));
            break;
        case 4:
            *pen_32 = color;
            break;
        default:
            fprintf(stderr, "Can't find the right size of pixel");
            break;
    }

}