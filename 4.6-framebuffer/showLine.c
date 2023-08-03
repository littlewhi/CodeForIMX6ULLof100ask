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
struct fb_var_screeninfo var;   /* Current var */
struct fb_fix_screeninfo fix;   /* Current fix */
int screen_size;
unsigned char *fbmem;
unsigned int line_width;
unsigned int pixel_width;
FT_Matrix matrix;

static int min(int a, int b) { return a > b ? b : a; }
static int max(int a, int b) { return a > b ? a : b; }

/* color : 0x00RRGGBB */
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
 * 修改日期        版本号     修改人        修改内容
 * -----------------------------------------------
 * 2020/05/12        V1.0     zh(angenao)         创建
 ***********************************************************************/ 
void
draw_bitmap( FT_Bitmap*  bitmap,
             FT_Int      x,
             FT_Int      y)
{
    FT_Int  i, j, p, q;
    FT_Int  x_max = x + bitmap->width;
    FT_Int  y_max = y + bitmap->rows;

    //printf("x = %d, y = %d\n", x, y);

    for ( j = y, q = 0; j < y_max; j++, q++ )
    {
        for ( i = x, p = 0; i < x_max; i++, p++ )
        {
            if ( i < 0      || j < 0       ||
                i >= var.xres || j >= var.yres )
            continue;

            //image[j][i] |= bitmap->buffer[q * bitmap->width + p];
            lcd_put_pixel(i, j, bitmap->buffer[q * bitmap->width + p]);
        }
    }
}

int compute_string_bbox(FT_Face face, wchar_t *wstr, FT_BBox  *abbox)
{
   
    FT_Vector pen;
    FT_Glyph glyph;
    FT_GlyphSlot slot = face->glyph;
    FT_BBox bbox, gbbox;
    int len = wcslen(wstr);
    int i;
    int error;

    pen.x = 0;
    pen.y = 0;

    bbox.xMin = bbox.yMin = 3200;
    bbox.xMax = bbox.yMax = -3200;

    for(i = 0; i < len; ++i)
    {
        /* 每个字的原点不一样 */      
        FT_Set_Transform(face, NULL, &pen);
        
        /* 加载位图 */
        error = FT_Load_Char(face, wstr[i], FT_LOAD_RENDER);

        /* 取出图像 */
        FT_Get_Glyph(face->glyph, &glyph);

        /* 取出本字（图像）bbox */
        FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_TRUNCATE, &gbbox);

        /* 比较最值 得到字符串外框 */        
        bbox.xMin = min(bbox.xMin, gbbox.xMin);
        bbox.xMax = max(bbox.xMax, gbbox.xMax);
        bbox.yMin = min(bbox.yMin, gbbox.yMin);
        bbox.yMax = max(bbox.yMax, gbbox.yMax);

        /* 更新原点 */
        pen.x += slot->advance.x;
        pen.y += slot->advance.y;
    }
    
    *abbox = bbox;

    return 0;
}


void displayString(FT_Face face, wchar_t *str, int lcdx, int lcdy)
{
    FT_BBox bbox;
    FT_GlyphSlot slot = face->glyph;
    FT_Vector pen;
    FT_Glyph glyph;
    int x, y, i;
    int len = wcslen(str);

    /* 计算外框 */
    compute_string_bbox(face, str, &bbox);

    /* 坐标体系的转换 */
    x = lcdx;
    y = var.yres - lcdy;

    /* 反推原点 */
    pen.x = (x - bbox.xMin) * 64; /* 单位: 1/64像素 */
    pen.y = (y - bbox.yMax) * 64; /* 单位: 1/64像素 */

    for(i = 0; i < len; ++i)
    {
        FT_Set_Transform(face, &matrix, &pen);

        FT_Load_Char(face, str[i], FT_LOAD_RENDER);

        draw_bitmap(&slot->bitmap, slot->bitmap_left, var.yres - slot->bitmap_top);

        pen.x += slot->advance.x;
        pen.y += slot->advance.y;
    }
    
}

int main(int argc, char **argv)
{

    double angle;
    int error;
    wchar_t *wstr = L"www.flySky飞行天空网.com\nkjdkejndkekf";
    wchar_t *next;
    wchar_t *pt = "111111111111";

    FT_Library    library;
    FT_Face       face;
   
    int font_size = 24;
    int lcd_x = strtoul(argv[2], NULL, 0), lcd_y = strtoul(argv[3], NULL, 0);


    if(argc < 5)
    {
        printf("Usage : %s <fontfile> <lcd_x> <lcd_y> <fontsize> [angle]\n", argv[0]);
        return 0;
    } 

    font_size = (int)strtoul(argv[4], NULL, 0);

    if(argc > 5)
        angle = strtoul(argv[5], NULL, 0) / 360.0 * 3.14159 * 2;
    
    fd_fb = open("/dev/fb0", O_RDWR);
    if(fd_fb < 0)
    {
        fprintf(stderr, "Cannot open\n");
        return -1;
    }

    if(ioctl(fd_fb, FBIOGET_VSCREENINFO, &var))
    {
        fprintf(stderr, "Cannot get io information\n");
        return -1;
    } 

    pixel_width = var.bits_per_pixel / 8;
    line_width = var.xres * pixel_width;
    screen_size = var.yres * line_width;

    fbmem = (unsigned char *) mmap(NULL, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);
    if(fbmem == (unsigned char *) -1)
    {
        fprintf(stderr, "Cannot map memory\n");
        return -1;
    }

    memset(fbmem, 0, screen_size);

    /* 矢量图部分 */

    /* 初始化库 */
    error = FT_Init_FreeType(&library);

    /* 打开格式文件 */
    error = FT_New_Face(library, argv[1], 0, &face);
    
    /* 设置大小 */
    error = FT_Set_Pixel_Sizes(face, font_size, 0);

    /* 设置旋转矩阵 */
    matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
	matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
	matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
	matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );
   
    next = wcstok(wstr, L"\n", &pt);
    for(printf("start\n"),fflush(stdout); next; next = wcstok(NULL, L"\n", &pt))
        //displayString(face, wstr, lcd_x, lcd_y);
        wprintf(next);
       printf("1"), fflush(stdout);

    munmap(fbmem, screen_size);
    close(fd_fb);
    
    return 0;   
}

