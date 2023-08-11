#include "asm-generic/fcntl.h"
#include "asm/gpio.h"
#include "asm/uaccess.h"
#include "linux/export.h"
#include "linux/fs.h"
#include "linux/slab.h"
#include "linux/kdev_t.h"
#include "linux/wait.h"
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
#include <linux/poll.h>
#include <linux/console.h>
#include <linux/types.h>
#include <linux/gpio/consumer.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>

/* 文件异步通知方式需要驱动提供drv_fasy(有内核自动根据用户app是否设置FASYNC调用)
 * 并且需要驱动主动发送信号给用户进程kill_fasync()
 */
static struct fasync_struct *fasync;
/* poll机制需要驱动提供drv_poll */
static DECLARE_WAIT_QUEUE_HEAD( yhb_key_poll_queue );

/* 阻塞式查询，需要一个等待队列 */
static DECLARE_WAIT_QUEUE_HEAD( yhb_key_wait_queue );

static struct class *cl;
static int major;


/* 数据缓冲区，环形队列 */
/* 操作函数不进行下标检查 */

struct yhb_key_t {
    short key;
    short key_data;
};

static int key_data_queue_empty(void);
static int key_data_queue_full(void);
static void key_data_queue_add(short, char);
static struct yhb_key_t key_data_queue_remove(void);

struct key_data_queue
{
#define YHB_BUFFER_SIZE 128
    int head, tail;
    struct yhb_key_t buf[YHB_BUFFER_SIZE];
    int (*empty)(void);
    int (*full)(void);
    void (*add)(short, char);
    struct yhb_key_t (*remove)(void);
} 
static dq = {
    .head = 0,
    .tail = 0,
    .empty = key_data_queue_empty,
    .full = key_data_queue_full,
    .remove = key_data_queue_remove,
    .add = key_data_queue_add
};

static int key_data_queue_empty( void )
{
    return dq.head == dq.tail;
}

static int key_data_queue_full( void )
{
    return dq.head == (dq.tail + 1) % YHB_BUFFER_SIZE;
}

static void key_data_queue_add( short key, char val )
{
    dq.buf[dq.tail].key_data = val;
    dq.buf[dq.tail].key = key;
    dq.tail = (dq.tail + 1)  % YHB_BUFFER_SIZE;
}

static struct yhb_key_t key_data_queue_remove( void )
{
    struct yhb_key_t val = dq.buf[dq.head++];
    dq.head %= YHB_BUFFER_SIZE;
    return val;
}

/* 返回的是一个长度为2的short数组，并且不区分哪个key，有数据就返回 */
static ssize_t yhb_key_read( struct file *filp, char __user * buf, size_t size, loff_t * lp )
{
    struct yhb_key_t val;
    /* 非阻塞式无数据 */
    if( (filp->f_flags & O_NONBLOCK) && dq.empty() )
    {
        return 0;
    }

    wait_event_interruptible( yhb_key_wait_queue, !dq.empty() );

    val = dq.remove();
    if( copy_to_user( buf, &val.key, sizeof(val.key) ) ||
     copy_to_user(buf + sizeof(val.key), &val.key_data, sizeof(val.key_data)))
     {
         return -EFAULT;
     }

     return sizeof(val.key_data) + sizeof(val.key);
}

/* 休眠机制有内核提供，驱动只需要挂载队列和提供返回状态 */
unsigned int yhb_key_poll( struct file *filp, struct poll_table_struct *wait )
{
    poll_wait(filp, &yhb_key_poll_queue, wait);

    /* 不为空时表示有内容可读 */
    return dq.empty() ? 0 : (POLLIN | POLLRDNORM);
}

/* 开启或关闭异步功能由内核进行调用 */
int yhb_key_fasync( int fd, struct file *filp, int on)
{
    return fasync_helper(fd, filp, on, &fasync);
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = yhb_key_read,
    .poll = yhb_key_poll,
    .fasync = yhb_key_fasync,
};


struct yhb_gpio_key
{
    int gpio;
    enum of_gpio_flags flags;
};

static struct yhb_gpio_key *yhb_gpio_keys;

static irqreturn_t yhb_key_interrupt( int irq, void *d )
{
    struct yhb_gpio_key *keys = (struct yhb_gpio_key *)d;
    int val = gpio_get_value( keys->gpio );
    printk( "key %d val is %d\n", keys->gpio,  val);

    dq.add( keys->gpio, val );
    wake_up_interruptible( &yhb_key_wait_queue );
    wake_up_interruptible( &yhb_key_poll_queue );
    kill_fasync( &fasync, SIGIO, POLL_IN );

    return IRQ_HANDLED;
} 

static int yhb_key_probe( struct platform_device *pdev )
{
    int count, i, gpio, irq;
    enum of_gpio_flags flags;
    struct device_node *node = pdev->dev.of_node;

    count = of_gpio_count( node );
    if( count == 0 )
    {
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -ENODEV;
    }
    yhb_gpio_keys = ( struct yhb_gpio_key* )kzalloc( sizeof(struct yhb_gpio_key) * count,  GFP_KERNEL );
    if( yhb_gpio_keys == NULL )
    {
        printk( "ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__ );
        return -1;
    }

    for( i = 0; i < count; ++i )
    {
        gpio = of_get_gpio_flags( node, i, &flags );
        if( !gpio_is_valid( gpio ) )
        {
            printk( "ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__ );
            return -1;
        }
        yhb_gpio_keys[i].gpio = gpio;
        yhb_gpio_keys[i].flags = flags;

        irq = gpio_to_irq( gpio );
        if( request_threaded_irq( irq, yhb_key_interrupt, NULL,
        IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "yhb_keys", &yhb_gpio_keys[i] ) )
        {
            printk( "ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__ );
            return -1;
        }
    }
    
    major = register_chrdev( 0, "yhb_key", &fops );
    if( major < 0 )
    {
        printk( "ERROR : Invalid major\n" );
        return -1;
    }
    cl = class_create( THIS_MODULE, "yhb_key" );
    if( IS_ERR( cl ) )
    {
        unregister_chrdev( major, "yhb_key" );
        printk( "ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__ );
        return PTR_ERR( cl );
    }

    if( IS_ERR( device_create( cl, NULL, MKDEV(major, 0), NULL, "yhb_key" ) ) )
    {
        class_destroy( cl );
        unregister_chrdev( major, "yhb_key" );
        printk( "ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__ );
        return -1;
    }
 
    return 0;
}

static int yhb_key_remove( struct platform_device *pdev )
{
    int count, i;
    struct device_node *node = pdev->dev.of_node;

    count = of_gpio_count( node );
    for( i = 0; i < count; ++i )
    {
        free_irq( gpio_to_irq( yhb_gpio_keys[i].gpio ), &yhb_gpio_keys[i] );
    }

    kfree( yhb_gpio_keys );

    device_destroy( cl,  MKDEV(major, 0) );
    class_destroy( cl );
    unregister_chrdev( major, "yhb_key" );

    return 0;
}

static const struct of_device_id yhb_key_ids[] = 
{
    { .compatible = "yhb_key" },
    { /* nil */ }
};

static struct platform_driver p_drv = 
{
    .probe = yhb_key_probe,
    .remove = yhb_key_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "yhb_key",
        .of_match_table = yhb_key_ids,
    }
};

static int __init yhb_key_init( void )
{
    return platform_driver_register( &p_drv );
}

static void __exit yhb_key_exit( void )
{
    return platform_driver_unregister( &p_drv );
}

module_init( yhb_key_init );
module_exit( yhb_key_exit );

MODULE_LICENSE("GPL");