#include "asm/uaccess.h"
#include "linux/export.h"
#include "linux/fs.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <linux/cpufreq.h>
#include <linux/firmware.h>
#include <linux/kthread.h>
#include <linux/regulator/driver.h>
#include <linux/fsl_devices.h>
#include <linux/ipu.h>
#include <linux/regmap.h>
#include <linux/pinctrl/consumer.h>
#include <linux/of_device.h>

#include <linux/console.h>
#include <linux/types.h>
#include <linux/gpio/consumer.h>

static struct gpio_desc *yhb_led_gpio_desc;
static struct class *yhb_led_class;
static unsigned int major;

static int yhb_led_open ( struct inode * ip, struct file *fp )
{
    /* 初始值是逻辑值， 0 - 关 */
    gpiod_direction_output( yhb_led_gpio_desc, 0 );
    return 0;
}

static int yhb_led_release ( struct inode * ip, struct file *fp )
{
    return 0;
}

static ssize_t yhb_led_write ( struct file *fp, const char __user *buf, size_t len, loff_t *lp )
{
    char status;
    if( copy_from_user( &status, buf, 1 ) )
    {
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__ );
        return -1;
    }

    /* 设置逻辑值， 1开0关 */
    gpiod_set_value( yhb_led_gpio_desc,  status ? 1 : 0 );
    return 1;
}

static struct file_operations fops = 
{
    .owner = THIS_MODULE,
    .open = yhb_led_open,
    .release = yhb_led_release,
    .write = yhb_led_write,
};



static int yhb_led_probe( struct platform_device *pdev )
{
    /* 获取gpio资源 */
    yhb_led_gpio_desc = gpiod_get( &pdev->dev, "yhb_led", 0 );
    if ( IS_ERR(yhb_led_gpio_desc) ) 
    {
        printk( "ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__ );
        return PTR_ERR( yhb_led_gpio_desc );
    }

    /* 注册字符设备 */
    if( (major = register_chrdev( 0, "yhb_led", &fops )) < 0 )
    {
        printk( "ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__ );
        return -1;
    }

    /* 创建类 */
    if( IS_ERR( (yhb_led_class = class_create(THIS_MODULE, "yhb_led")) ) )
    {
        printk( "ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__ );
        return -1;
    }

    /* 创建设备文件 */
    if( IS_ERR( device_create(yhb_led_class, NULL, MKDEV(major, 0), NULL, "yhb_led") ) )
    {
        printk( "ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__ );
        return -1;
    }

    return 0;
}

static int yhb_led_remove( struct platform_device *pdev )
{
    /* 反序销毁注册信息和资源 */
    device_destroy(yhb_led_class, MKDEV(major, 0) );
    class_destroy( yhb_led_class );
    unregister_chrdev( major, "yhb_led" );
    gpiod_put( yhb_led_gpio_desc );

    return 0;
}

static const struct of_device_id yhb_led_ids[] = 
{
    { .compatible = "yhb_led" },
    { /* nil */ }
};

static struct platform_driver p_drv = 
{
    .probe = yhb_led_probe,
    .remove = yhb_led_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "yhb_led",
        .of_match_table = yhb_led_ids,
    }
};

static int __init yhb_led_init( void )
{
    return platform_driver_register( &p_drv );
}

static void __exit yhb_led_exit( void )
{
    return platform_driver_unregister( &p_drv );
}

module_init( yhb_led_init );
module_exit( yhb_led_exit );

MODULE_LICENSE("GPL");