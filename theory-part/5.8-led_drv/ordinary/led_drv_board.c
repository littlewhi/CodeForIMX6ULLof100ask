#include "led_drv_board.h"
#include "led_drv_resource.h"
#include "linux/printk.h"

static struct led_resource *led = NULL;

/* 不做太多算法，我只找到一个可用在gpio的led灯, 通用操作与资源的划分还是要具体情况具体分析 */
   
/* 虚拟地址 */   
//static unsigned int *reg_out; /* 输出数据寄存器 */
//static unsigned int *reg_in; /* 输入数据寄存器 */
//static unsigned int *reg_dir; /* 设置是输入还是输出方向的寄存器 */
//static unsigned int *reg_mux; /* 功能模式选择寄存器 */
//static unsigned int *reg_enable; /* 使能寄存器 */


/* 设置模式gpio */
static int led_open(short group, short pin)
{
    int i;
    struct led_resource *leds;
    printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    /* 寻找正确的引脚 */
    leds = get_led_resource_array();
    for(i = 0; leds[i].group != -1; ++i)
    {
        if(leds[i].group == group && leds[i].pin == pin) /* 找到了 */
        {
            led = leds + i;
            break;
        }
    }

    if(led == NULL) /* 没有找到 */
        return -1;


    /* 初始化寄存器 */
    if(led->setReg(led))
    {
        printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    if(led->enableModule(led))
    {
        printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    if(led->selectMux(led))
    {
        printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    return 0;
}

static int led_close(void)
{
    if(led->unsetReg(led))
    {
       printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
       return -1; 
    }

    return 0;
}

static int led_write(char val)
{
    if(led->setDir('1', led) || !led->putOut(val, led))
    {
        printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 1;
}

static int led_read(char *buf, size_t len)
{
    if(led->setDir(0, led) || !led->getIn(buf, led, len))
    {
        printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    return 1;
}


static struct led_operations led_op = 
{
    .num = NUM_LED,
    .open = led_open,
    .write = led_write,
    .read = led_read,
    .close = led_close,
};

struct led_operations * get_led_operations(void)
{
    return &led_op;
}