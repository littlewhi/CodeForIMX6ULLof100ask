

#include "linux/fs.h"
#include "linux/module.h"
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/raw.h>
#include <linux/tty.h>
#include <linux/capability.h>
#include <linux/ptrace.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/backing-dev.h>
#include <linux/shmem_fs.h>
#include <linux/splice.h>
#include <linux/pfn.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/uio.h>

#include <linux/uaccess.h>
#include "button_opt.h"


static struct class *but_class;
static unsigned int major;
static struct button_operations *bops;

int open (struct inode *np, struct file *fp)
{
    int minor = iminor(np);
    return bops->open(minor);
}

ssize_t read (struct file *fp, char __user *buf, size_t size, loff_t *lp)
{
    int val, error, minor;

    minor = iminor(file_inode(fp));
    val = bops->read(minor);

    if(val < 0)
    {
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    if(copy_to_user(buf, &val, 1))
    {
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    return 1;
}

int close (struct inode *inode, struct file *fp)
{
    int minor = iminor(inode);
    return bops->close(minor);
}

static struct file_operations fops = 
{
    .open = open,
    .read = read,
    .release = close
};

static int __init button_drv_init(void)
{
    major = register_chrdev(0, "yhb_button", &fops);
    if(major <= 0)
    {
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    but_class = class_create(THIS_MODULE, "yhb_button");
    if(!but_class)
    {
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    return 0;

}



static void __exit button_drv_exit(void)
{
    class_destroy(but_class);
    unregister_chrdev(major, "yhb_button");
}


int button_device_create(int minor)
{
    if(!device_create(but_class, NULL, MKDEV(major, minor), NULL, "yhb_button%d", minor))
    {
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

void button_device_destroy(int minor)
{
    device_destroy(but_class, MKDEV(major, minor));
}

void get_button_operations_pointer(struct button_operations *p)
{
    bops = p;
}

EXPORT_SYMBOL(button_device_create);
EXPORT_SYMBOL(button_device_destroy);
EXPORT_SYMBOL(get_button_operations_pointer);

module_init(button_drv_init);
module_exit(button_drv_exit);
MODULE_LICENSE("GPL");