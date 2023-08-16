#include <linux/module.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/platform_device.h>
#include <asm/io.h>

#include <linux/module.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/platform_device.h>
#include <linux/mm.h>


#ifndef BOARD_ASK_LED_OPT_H
#define BOARD_ASK_LED_OPT_H
struct led_operations
{
    short cur_group;
    short cur_pin;
    int (*write)(char); /* write */
    int (*read)(char *, unsigned int); /* read */
    int (*open)(int); /* open */
    int (*close)(void); /* close */
};

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

#endif