
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
#include <linux/errno.h>	
#include <linux/ioport.h>	
#include <linux/delay.h>	
#include <linux/sched.h>
#include <linux/mutex.h>
#include <asm/io.h>		
#include <asm/uaccess.h>	
#include <linux/wait.h>		
#include <linux/init.h>		

#include "board_ask_led_opt.h"

static struct class *led_class;
static struct device *led_device;
static unsigned int major;

/* 硬件操作第二层的接口 */
static struct led_operations *led_op;

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

/* 
 * 设备文件的创建在
 * struct platform_device和
 * struct platfomr_driver匹配成功
 * 调用probe()时进行注册，
 * 因为那是才知道具体的led有多少。 
 */
static int led_device_create(int minor)
{
    led_device = device_create(led_class, NULL, MKDEV(major, minor), NULL, "yhb_led%d", minor);
    if(IS_ERR(led_device))
    {
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    return 0;
}
EXPORT_SYMBOL(led_device_create);

/* 
 * 删除也是在移除bus匹配时进行
 */
static int led_device_remove(int minor)
{
    device_destroy(led_class, MKDEV(major, minor));
    return 0;
}
EXPORT_SYMBOL(led_device_remove);

/*  
 * 为了解决依赖关系，有第二层调用
 */
static int get_led_operations(struct led_operations *led_p)
{
    led_op =  led_p;
    return 0;
}
EXPORT_SYMBOL(get_led_operations);


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

    /* 设置gpio模式 */
    if(led_op->open(minor))
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

    major = register_chrdev(0, "yhb_led", &led_fops);
    if(major < 0)
    {
        printk(KERN_ERR "register_chrdev failed\n");
        printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    led_class = class_create(THIS_MODULE, "yhb_led");
    if(IS_ERR(led_class))   
    {
        unregister_chrdev(major, "yhb_led");
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    return 0;
}

static void __exit led_cleanup (void)
{ 
    class_destroy(led_class);
    unregister_chrdev(major, "yhb_led");
}

module_init(led_init);
module_exit(led_cleanup);
MODULE_LICENSE("GPL");


