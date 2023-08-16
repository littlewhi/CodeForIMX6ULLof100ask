
#include "led_drv_board.h"

#include "linux/buffer_head.h"
#include "linux/device.h"
#include "linux/export.h"
#include "linux/kdev_t.h"
#include "linux/spi/spi.h"
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

static struct class *led_class;
static struct device *led_device;
static unsigned int major;

static struct led_operations *led_op;

static ssize_t led_read(struct file *, char __user *, size_t nbytes, loff_t * ppos);
static ssize_t led_write(struct file *, const char __user *, size_t nbytes, loff_t * ppos);
static int led_open(struct inode *, struct file *);
static int led_release(struct inode *, struct file *);

/* 将次设备号作为索引寻找对应引脚， 每一个元素高16位表示组，低16位表示引脚 */
static int led_nodes[20] = {(5 <<16 ) | (3), 0};
static int lenOflnodes = 1;
#define GET_GROUP(x) ((x) >> 16)
#define GET_PIN(x) ((x) & 0x0000ffff)

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
    char val;
    int ret;
    if(!led_op->read(&val, sizeof(val)))
    {
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return 0;
    }
    
    ret = copy_to_user(buf, &val, sizeof(val));
    if(ret)
    {
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return 0;
    }

    return 1;
}

static ssize_t led_write(struct file *fp, const char __user * buf, size_t nbytes, loff_t * ppos)
{
    char val;
    if(copy_from_user(&val, buf, sizeof(val)))
    {
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return 0;
    } 

    if(!led_op->write(val))
    {
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return 0;
    }

    return 1;
}

/* iminor(struct inode *) -> 根据inode得到次设备节点
 * imajor(struct inode *) -> 根据inode得到主设备节点
 */
static int led_open(struct inode * ip, struct file *fp)
{
    int minor;
    printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    minor = iminor(ip);
    printk("%s %s %d minor = %d\n", __FILE__, __FUNCTION__, __LINE__, minor);
    if (minor >= lenOflnodes)
		return -1;
        printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    /* 设置gpio模式 */
    if(led_op->open(GET_GROUP(led_nodes[minor]), GET_PIN(led_nodes[minor])))
    {
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    return 0;
}

static int led_release(struct inode *ip, struct file *fp)
{
    return led_op->close();
}

static int __init led_init(void)
{
    int i;

    major = register_chrdev(0, "yhb_led", &led_fops);
    if(major < 0)
    {
        printk(KERN_ERR "register_chrdev failed\n");
        printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    led_op = get_led_operations();

    led_class = class_create(THIS_MODULE, "yhb_led");
    if(IS_ERR(led_class))   
    {
        unregister_chrdev(major, "yhb_led");
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    for(i = 0; i < led_op->num; ++i)
    {
        led_device = device_create(led_class, NULL, MKDEV(major, i), NULL, "yhb_led%d", i);
        if(IS_ERR(led_device))
        {
            printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
    }
    
    return 0;
}

static void __exit led_cleanup (void)
{
    int i;
    for(i = 0; i < led_op->num; ++i)
    {
        device_destroy(led_class, MKDEV(major, i));
    }
    
    class_destroy(led_class);
    unregister_chrdev(major, "yhb_led");
}


module_init(led_init);
module_exit(led_cleanup);
MODULE_LICENSE("GPL");


