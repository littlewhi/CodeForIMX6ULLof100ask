#ifndef _BUTTON_OPT_H
#define _BUTTON_OPT_H

#define MLEN 32
struct button_operations
{
    int (*open)(int);
    int (*close)(int);
    int (*read)(int); /* 返回值作为读入，1为按下，0为正常， -1为错误 */
};

/* gpio5_1 0x20AC000, ccgr1-cg15 : 0x20C406C, snvs_tamper1 : 0x229000C 第五位-input - 1 force,低四位 mode-alt5*/
/* gpio4_14 0x20A8000, ccgr3-cg6 :  0x20C4074, snvsNAND_CE1_B : 0x20E01B0 mode-alt5*/
struct yhb_gpio_addr
{
    volatile unsigned int dr; /* data of outing*/
    volatile unsigned int gdir; /* pin's direction, 0-input, 1-output */
    volatile unsigned int psr; /* pin's status */
    volatile unsigned int pcr1; /* interrupt configuration */
    volatile unsigned int pcr2; /* interrupt configuration */
    volatile unsigned int imr; /* interrup mask */
    volatile unsigned int isr; /* interrup status */
    volatile unsigned int edge_sel; /* edge select */
};

struct yhb_gpio_operations
{
    int (*enable)(struct yhb_gpio_addr *, unsigned short group, unsigned int pin);
    int (*setMode)(struct yhb_gpio_addr*, unsigned short group, unsigned int pin);
    int (*setDirection)(struct yhb_gpio_addr*, unsigned short group, unsigned int pin);
    int (*input)(struct yhb_gpio_addr *, unsigned short group, unsigned int pin); /* 0 - pressed */
};


int button_device_create(int minor);
void button_device_destroy(int minor);
void get_button_operations_pointer(struct button_operations *p);
struct button_operations* get_button_operations_pointer_from_opt(void);
unsigned int *get_minor_to_pin(void);

#endif