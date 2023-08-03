

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



static unsigned int major;
static unsigned int minor;
static struct class *led_class;
static struct device *led_device;

static volatile unsigned int *GPIO5_DR; /* gpio数据寄存器（20A_C000） */
static volatile unsigned int *GPIO5_GDIR;/* gpio方向寄存器（20A_C004）*/
static volatile unsigned int *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3;/* iomuxc模式选择寄存器，（229_0014） */
static volatile unsigned int *GPIO5_PSR; /* gpio输入获取寄存器, (20A_C008) */


static ssize_t led_read(struct file *, char __user *, size_t nbytes, loff_t * ppos);
static ssize_t led_write(struct file *, const char __user *, size_t nbytes, loff_t * ppos);
static int led_open(struct inode *, struct file *);
static int led_release(struct inode *, struct file *);

static const struct file_operations led_fops =
{
	.owner		= THIS_MODULE,
	.write		= led_write,
    .read       = led_read,
	.open		= led_open,
	.release	= led_release,
};

static ssize_t led_read(struct file *fp, char __user *buf, size_t nbytes, loff_t * ppos)
{
    char ret;

    if(nbytes == 0)
        return 0;
    
     /* 设置gpio方向 输入 */
    *GPIO5_GDIR &= ~(1 << 3);

    /* return_buf : 1 - on, 0 - off */
    ret = *GPIO5_PSR & (1 << 3) ? 0 : 1;
    if(copy_to_user(buf, &ret, 1))
    {
        printk("Error : copy_to_user failed\n");
        return 0;
    }

    return 1;
}

static ssize_t led_write(struct file *fp, const char __user * buf, size_t nbytes, loff_t * ppos)
{
    char val;
    
    /* 设置gpio方向 输出 */
    *GPIO5_GDIR |= (1 << 3);
    
    if(copy_from_user(&val, buf, 1))
    {
        printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return 0;
    }

    if(val)/* 非零值就点亮LED */
    {
        *GPIO5_DR &= ~(1 << 3);
    }
    else
    {
        *GPIO5_DR |= (1 << 3);
    }

    return 1;
}


static int led_open(struct inode * ip, struct file *fp)
{
    /* 设置gpio模式 */
    *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 &= ~0xf;
    *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 |= 0x5;
    *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 |= (1 << 4); /* 设置回环输入 */
    
    return 0;
}

static int led_release(struct inode *ip, struct file *fp)
{
    return 0;
}

static int __init led_init(void)
{

    GPIO5_DR = ioremap(0x20AC000, 4); /* 数据 */
    if(GPIO5_DR == NULL)
    {
        printk("ERROR: %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    GPIO5_GDIR = ioremap(0x20AC004, 4); /* 方向 */
    if(GPIO5_GDIR == NULL)
    {
        printk("ERROR: %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 = ioremap(0x2290014, 4); /* 模式选择 */
    if(IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 == NULL)
    {
        printk("ERROR: %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    GPIO5_PSR = ioremap(0x20AC008, 4);
    if(GPIO5_PSR == NULL)
    {
        printk("ERROR: %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    /* 注册字符设备， 初始major为零有系统自动分配主设备号 */
    major = register_chrdev(major, "myled", &led_fops);
    minor = 0;
    if(major < 0)
    {
        printk("ERROR: %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        printk("ERROR: System cannot register led device\n");
        return -1;
    }

    /* 创建设备信息类 */
    led_class = class_create(THIS_MODULE, "myled");
    if (IS_ERR(led_class)) 
    {
        printk("ERROR: %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		printk("ERROR: Could not create led device class\n");
        unregister_chrdev(major, "myled");
        return -1;
	}

    /* 创建设备文件 */
    led_device = device_create(led_class, NULL, MKDEV(major, minor), NULL, "myled");
    if(IS_ERR(led_device))
    {
        printk("ERROR: %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        printk("Error creating device for device %d\n", MKDEV(major, minor));
        class_destroy(led_class);
        unregister_chrdev(major, "myled");
        return -1;
    }
    return 0;
}

static void __exit led_cleanup (void)
{
    iounmap(GPIO5_DR);
    iounmap(GPIO5_GDIR);
    iounmap(IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3);
    iounmap(GPIO5_PSR);

	device_destroy(led_class, MKDEV(major, minor));
    class_destroy(led_class);
    unregister_chrdev(major, "myled");
}


module_init(led_init);
module_exit(led_cleanup);
MODULE_LICENSE("GPL");
