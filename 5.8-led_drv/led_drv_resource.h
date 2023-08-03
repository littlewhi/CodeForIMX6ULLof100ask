/* 定义资源格式 */
#ifndef LED_RESOURCE_H
#define LED_RESOURCE_H


#include "linux/buffer_head.h"
#include "linux/device.h"
#include "linux/export.h"
#include "linux/kdev_t.h"
#include "linux/stddef.h"
#include <linux/module.h>

#include <linux/types.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/errno.h>	/* for -EBUSY */
#include <linux/ioport.h>	/* for request_region */
#include <linux/delay.h>	/* for loops_per_jiffy */
#include <linux/sched.h>
#include <linux/mutex.h>
#include <asm/io.h>		/* for inb_p, outb_p, inb, outb, etc. */
#include <asm/uaccess.h>	/* for get_user, etc. */
#include <linux/wait.h>		/* for wait_queue */
#include <linux/init.h>		/* for __init, module_{init,exit} */


struct led_resource
{
    short group; /* 哪个gpio组 */
    short pin;  /* gpio组的哪个引脚 */

    volatile unsigned int *reg_out; /* 输出数据寄存器 */
    volatile unsigned int *reg_in; /* 输入数据寄存器 */
    volatile unsigned int *reg_dir; /* 设置是输入还是输出方向的寄存器 */
    volatile unsigned int *reg_mux; /* 功能模式选择寄存器 */
    volatile unsigned int *reg_enable; /* 使能寄存器 */

    int (*setReg)(struct led_resource*); /* 设置寄存器的虚拟地址 */
    int (*enableModule)(struct led_resource*); /* 使能 */
    int (*setDir)(char, struct led_resource*); /* 设置方向 */
    int (*selectMux)(struct led_resource*); /* gpio模式选择 */
    int (*putOut)(char, struct led_resource*); /* 输出数据 */
    int (*getIn)(char*, struct led_resource*, unsigned int); /* 获取输入 */
    int (*unsetReg)(struct led_resource*); /* 释放虚拟地址 */
};

struct led_resource *get_led_resource_array(void);

#endif