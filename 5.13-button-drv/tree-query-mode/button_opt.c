
#include "button_opt.h"
#include <asm/io.h>

#define YHB_GPIO_5 5
#define YHB_GPIO_4 4
#define YHB_GPIO_3 3
#define YHB_GPIO_2 2
#define YHB_GPIO_1 1
#define YHB_GPIO_0 0
#define YHB_GPIO_MAX 32
#define GET_GPIO_GROUP(x) (((x) >> 16) & 0xffff)
#define GET_GPIO_PIN(x) ((x) & 0xffff)
#define GPIO_BASE_ADDR 0x2098000
#define GPIO_GAP (1 << 14)

unsigned int minorTopin[32];

static struct yhb_gpio_addr* gpio_addr[YHB_GPIO_MAX];

static int enable(struct yhb_gpio_addr *p, unsigned short group, unsigned int pin)
{
    volatile unsigned int *enable_reg;
    switch (group)
    {
        case YHB_GPIO_5:
            enable_reg = ioremap(0x20C406C, 4);
            *enable_reg |= (3<<(15 * 2));
           
            break;
        case YHB_GPIO_4:
            enable_reg = ioremap(0x20C4074, 4);
            *enable_reg |= (3<<(6 * 2));
            break;
        default:
            printk("Not find right pin for you gpio device\n");
            return -1;
    } 

    iounmap(enable_reg);
    return 0;
}

static int setMode(struct yhb_gpio_addr* p, unsigned short group, unsigned int pin)
{
    volatile unsigned int *selectMode_reg;
    unsigned int val;
    switch (group)
    {
        case YHB_GPIO_5:
            selectMode_reg = ioremap(0x229000C, 4);
            break;
        case YHB_GPIO_4:
            selectMode_reg = ioremap(0x20E01B0, 4);
            break;
        default:
            printk("Not find right pin for you gpio device\n");
            return -1;
    }

    val = *selectMode_reg;
    val &= ~0xf;
    val |= 5 + 16;
    *selectMode_reg = val;
    iounmap(selectMode_reg);
    return 0;
}

static int setDirection(struct yhb_gpio_addr *p, unsigned short group, unsigned int pin)
{
    p->gdir &= ~(1 << pin);
    return 0;
}

static int input(struct yhb_gpio_addr *p, unsigned short group, unsigned int pin)
{
    if(p->psr & (1 << pin))
        return 0;
    
    return 1;
}
/*
static struct yhb_gpio_operations g_ops = 
{
    .enable = enable,
    .input = input,
    .setDirection = setDirection,
    .setMode = setMode
};
*/
/****************************************************************/


static int open(int minor)
{
    int g, p;
    g = GET_GPIO_GROUP(minorTopin[minor]);
    p = GET_GPIO_PIN(minorTopin[minor]);

    gpio_addr[g]= ioremap(GPIO_BASE_ADDR + g * GPIO_GAP, sizeof(struct yhb_gpio_addr));
    if(gpio_addr[g] == NULL)
    {
        printk("ioremap failed\n");
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    if(enable(gpio_addr[g], g, p))
        return -1;
    setMode(gpio_addr[g], g, p);
    setDirection(gpio_addr[g], g, p);

    return 0;
}

static int close(int minor)
{
    int g, p;
    g = GET_GPIO_GROUP(minorTopin[minor]);
    p = GET_GPIO_PIN(minorTopin[minor]);

    iounmap(gpio_addr[g]);
    return 0;
}

/* 返回值为1代表按下 */
static int read(int minor)
{
    int g, p;
    g = GET_GPIO_GROUP(minorTopin[minor]);
    p = GET_GPIO_PIN(minorTopin[minor]);

    return input(gpio_addr[g], g, p) ? 0 : 1;
}

static struct button_operations b_ops= 
{
    .open = open,
    .close = close,
    .read = read
};

struct button_operations *get_button_operations_pointer_from_opt(void)
{
    return &b_ops;
}

unsigned int* get_minor_to_pin(void)
{
    return minorTopin;
}