
#include "board_ask_led_opt.h"

#include "linux/export.h"
#include "linux/percpu-refcount.h"
#include "linux/platform_device.h"

static struct led_resource *cur_led; /* 第二层使用的通用指针 */

static unsigned int minorTopin[32]; /* minor to group and pin */
static int len;

extern int led_device_create(int);
extern int led_device_remove(int);
extern int get_led_operations(struct led_operations *);

struct led_resource* get_led_resource(int, int);

#define GET_GROUP(x) (((x) >> 16) & 0xffff)   /* 获取组号 */
#define GET_PIN(x) ((x) & 0xffff)       /* 获取引脚号 */

/* 
 * 硬件操作二三层和struct platform_driver在一个源文件实现(省事)
 */

/* 
 * 硬件操作二层 -> struct led_operations
 */

/* 
 * 第二层调用的，用来获取第三层的对应的led的struct led_resource*
 */
static struct led_resource *init_led(int group, int pin)
{
    printk("INIT : GPIO group %d, pin %d\n", group, pin);

    return get_led_resource(group, pin);
}

/* 设置模式gpio */
static int led_open(int minor)
{
    int group, pin;

    printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    
    group = GET_GROUP(minorTopin[minor]);
    pin = GET_PIN(minorTopin[minor]);

    cur_led = init_led(group, pin);
    if(cur_led->group != 5 || cur_led->pin != 3)
    {
        printk("GPIO: group %d, pin %d\n", group, pin);
        return 0;
    }

    /* 初始化寄存器 */
    if(cur_led->setReg(cur_led))
    {
        printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    if(cur_led->enableModule(cur_led))
    {
        printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    if(cur_led->selectMux(cur_led))
    {
        printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    return 0;
}

static int led_close(void)
{
    if(cur_led->group != 5 || cur_led->pin != 3)
    {
        printk("GPIO group %d pin %d is closed\n", cur_led->group, cur_led->pin);
        return 0;
    }
    if(cur_led->unsetReg(cur_led))
    {
       printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
       return -1; 
    }

    return 0;
}

static int led_write(char val)
{
    if(cur_led->group != 5 || cur_led->pin != 3)
    {
        printk("GPIO group %d pin %d is %s\n", cur_led->group, cur_led->pin, val ? "on" : "off");
        return 1;
    }
    if(cur_led->setDir('1', cur_led) || !cur_led->putOut(val, cur_led))
    {
        printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 1;
}

static int led_read(char *buf, size_t len)
{
    
    if(cur_led->group != 5 || cur_led->pin != 3)
    {
        printk("GPIO grup %d pin %d is in status\n", cur_led->group, cur_led->pin);
        *buf = 0;
        return 1;
    }

    if(cur_led->setDir(0, cur_led) || !cur_led->getIn(buf, cur_led, len))
    {
        printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    return 1;
}

static struct led_operations led_op = 
{
    .open = led_open,
    .write = led_write,
    .read = led_read,
    .close = led_close,
};

/* 
 * bus -> struct platform_driver的实现
 */

static int askled_probe(struct platform_device *dev)
{
    int i, group, pin;
    struct resource *led;
    len = dev->num_resources;
    for(i = 0; i < len; ++i)
    {
        led = platform_get_resource(dev, IORESOURCE_MEM, i);
        if(led == NULL)
        {
            printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
            return -ENODEV;
        }

        /* minor与硬件建立联系 */
        sscanf(led->name, "%d$%d", &group, &pin);
        minorTopin[i] = (group << 16) | pin;
        printk("group = %d\npin = %d\n", group, pin);
        
        /* 创建led文件 */
        led_device_create(i);
    }
    get_led_operations(&led_op);
    return 0;
}

static int askled_remove(struct platform_device *dev)
{
    int i;
    for(i = 0; i < len; ++i)
    {
        led_device_remove(i);
    }

    return 0;
}

static struct platform_driver askled_opt = 
{
    .probe = askled_probe,
    .remove = askled_remove,
    .driver = {
        .name = "100ask_led",
        .owner = THIS_MODULE
    }
};

static int __init askled_opt_init(void)
{
    int error;
    error = platform_driver_register(&askled_opt);
    if(error)
    {
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return error;
    }

    return 0;
}

static void __exit askled_opt_exit(void)
{
    return platform_driver_unregister(&askled_opt);
}

module_init(askled_opt_init);
module_exit(askled_opt_exit);
MODULE_LICENSE("GPL");


/* 
 * 硬件操作第三层 struct led_resource 
 */
 
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

struct led_resource *get_led_resource(int group, int pin)
{
     if(group == 5 && pin == 3)
          return leds;
     else
     {
        leds[1].group = group;
        leds[1].pin = pin;
        return leds + 1;
     }
}
