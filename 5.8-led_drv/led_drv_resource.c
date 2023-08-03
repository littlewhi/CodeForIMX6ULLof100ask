#include "led_drv_resource.h"
#include "asm/io.h"
#include "linux/printk.h"

/* 定义具体资源， 当更换添加同板led时， 修改此文件的函数与数据结构 */

#define GET_GROUP(x) ((x)->group)   /* 获取组号 */
#define GET_PIN(x) ((x)->pin)       /* 获取引脚号 */

static int setReg(struct led_resource *p) /* 设置寄存器的虚拟地址 */
{
     printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

     p->reg_enable = ioremap(0x20C406C, 4);
     p->reg_mux = ioremap(0x2290014, 4);
     p->reg_dir = ioremap(0x20ac004, 4);
     p->reg_in = ioremap(0x20ac008, 4);
     p->reg_out = ioremap(0x20ac000, 4);
     
     if(!p->reg_dir || !p->reg_in || !p->reg_out || !p->reg_enable || !p->reg_mux)
     {
          printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
          return -1;
     }

     return 0;
}

static int enableModule(struct led_resource *p) /* 使能 */
{
     printk("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);

     *p->reg_enable |= (3 << 30);
     return 0;
}

/* val非零就输出 */
static int setDir(char val, struct led_resource *p) /* 设置方向 */
{ 
     int dir;
     printk("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
     
     dir = val ? 1 : 0;
     *p->reg_dir |= (dir << 3);

     return 0;
}

static int selectMux(struct led_resource *p) /* gpio模式选择 */
{
     int val;
     printk("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
     
     val = *p->reg_mux;
     val &= ~(0xf);
     val |= 5;
     val |= (1 << 4);
     *p->reg_mux = val;
     
     return 0;
}
static int putOut(char data, struct led_resource *p) /* 输出数据 */
{
     printk("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);

     if(!data)
     {
          *p->reg_out |= (1 << 3);
     }
     else
     {
          *p->reg_out &= ~(1 << 3);
     }

     return 1;
}
static int getIn(char* buf, struct led_resource *p, unsigned int len) /* 获取输入 */
{
     printk("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);

     *buf = !((*p->reg_in >> 3) & 1);

     return 1;
}

static int unsetReg(struct led_resource *p) /* 释放虚拟地址 */
{
     printk("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);

     iounmap(p->reg_in);
     iounmap(p->reg_out);
     iounmap(p->reg_mux);
     iounmap(p->reg_dir);
     iounmap(p->reg_enable);
     
     return 0;
}
static struct led_resource leds[] = 
{
   {
        .group = 5,
        .pin = 3,
        /*.reg_out = 0x20AC000,
        .reg_in = 0x20AC008,
        .reg_dir = 0x20AC004,
        .reg_mux = 0x2290014,
        .reg_enable = */ 
        .setReg = setReg,
        .enableModule = enableModule,
        .setDir = setDir,
        .getIn = getIn,
        .putOut = putOut,
        .selectMux = selectMux,
        .unsetReg = unsetReg
   },
   {
        .group = -1,
        .pin = -1,
   }
};


/* 返回led资源数组，用.group = -1 作为结束, 当然，led_operation结构体中的定义也是个数 */
struct led_resource *get_led_resource_array(void)
{
    return leds;
}
