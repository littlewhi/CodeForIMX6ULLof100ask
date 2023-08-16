#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <wchar.h>
#include <sys/ioctl.h>
#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_GLYPH_H

int fd_fb;
struct fb_var_screeninfo var;	/* Current var */
int screen_size;
unsigned char *fbmem;
unsigned int line_width;
unsigned int pixel_width;


/**********************************************************************
 * 函数名称： lcd_put_pixel
 * 功能描述： 在LCD指定位置上输出指定颜色（描点）
 * 输入参数： x坐标，y坐标，颜色
 * 输出参数： 无
 * 返 回 值： 会
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2020/05/12	     V1.0	  zh(angenao)	      创建
 ***********************************************************************/ 
void lcd_put_pixel(int x, int y, unsigned int color)
{
	unsigned char *pen_8 = fbmem+y*line_width+x*pixel_width;
	unsigned short *pen_16;	
	unsigned int *pen_32;	

	unsigned int red, green, blue;	

	pen_16 = (unsigned short *)pen_8;
	pen_32 = (unsigned int *)pen_8;

	switch (var.bits_per_pixel)
	{
		case 8:
		{
			*pen_8 = color;
			break;
		}
		case 16:
		{
			/* 565 */
			red   = (color >> 16) & 0xff;
			green = (color >> 8) & 0xff;
			blue  = (color >> 0) & 0xff;
			color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
			*pen_16 = color;
			break;
		}
		case 32:
		{
			*pen_32 = color;
			break;
		}
		default:
		{
			printf("can't surport %dbpp\n", var.bits_per_pixel);
			break;
		}
	}
}

/**********************************************************************
 * 函数名称： draw_bitmap
 * 功能描述： 根据bitmap位图，在LCD指定位置显示汉字
 * 输入参数： x坐标，y坐标，位图指针
 * 输出参数： 无
 * 返 回 值： 无
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2020/05/12	     V1.0	  zh(angenao)	      创建
 ***********************************************************************/ 

void
draw_bitmap( FT_Bitmap*  bitmap,
             FT_Int      x,
             FT_Int      y)
{
	FT_Int fti, ftj, fbi, fbj,
		x_max = x + bitmap->width,
		y_max = y + bitmap->rows;

	for(fti = 0, fbi = x; fbi < x_max; ++fti, ++fbi)
	{
		for(ftj = 0, fbj = y; fbj < y_max; ++ftj, ++fbj)
		{
			if(fbi >= var.xres || fbj >= var.yres || fbi < 0 || fbj < 0)
			{
				continue;
			}
			lcd_put_pixel(fbi, fbj, bitmap->buffer[fti + ftj * bitmap->width]);
		}
	}
}


int main(int argc, char **argv)
{

	wchar_t *str = L"贰";

	FT_Library library;
	FT_Face face;
	int error;
	FT_Vector pen;
	FT_GlyphSlot slot;
	int fontSize = 24;

	if(argc < 2)
	{
		printf("usage : <formatfile> [size]");
		return -1;
	}
	if(argc == 3)
		fontSize = strtoul(argv[2], NULL, 0);

	/* 打开framebuffer并map*/	
	fd_fb = open("/dev/fb0", O_RDWR);
	if(fd_fb < 0)
	{
		fprintf(stderr, "Cannot open /dev/fb0\n");
		return -1;
	}

	if(ioctl(fd_fb, FBIOGET_VSCREENINFO, &var))
	{
		fprintf(stderr, "Cannot get infomation of /dev/fb0\n");
		return -1;
	}

	pixel_width = var.bits_per_pixel / 8;
	line_width = var.xres * pixel_width;
	screen_size = var.yres * line_width;

	/* void *mmap(void *addr, size_t length, int prot, int flags,
                  int fd, off_t offset);
       int munmap(void *addr, size_t length); */
	
	fbmem = (unsigned char *)mmap(NULL, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);
	if(fbmem == (unsigned char*)-1)
	{
		fprintf(stderr, "Cannot map memory for /dev/fb0\n");
		return -1;
	}

	memset(fbmem, -1, screen_size);/* 将屏幕置白*/

	/* 矢量图的显示 */

	/* 初始化 freetype库*/
	error = FT_Init_FreeType(&library);

	/* 初始化face(打开字体格式文件) */
	error = FT_New_Face(library, argv[1], 0, &face);

	slot = face->glyph;

	/* 设置大小 */
	FT_Set_Pixel_Sizes(face, fontSize, 0);

	/* 获取位图 */
	error = FT_Load_Char(face, str[0], FT_LOAD_RENDER);

	draw_bitmap(&slot->bitmap, var.xres / 2, var.yres / 2);

	return 0;	
}

