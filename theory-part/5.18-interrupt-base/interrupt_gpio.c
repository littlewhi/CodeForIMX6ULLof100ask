#include "asm/gpio.h"
#include "asm/uaccess.h"
#include "linux/export.h"
#include "linux/fs.h"
#include "linux/slab.h"
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
#include <linux/of_gpio.h>
#include <linux/slab.h>

struct yhb_gpio_key
{
    int gpio;
    enum of_gpio_flags flags;
};

static struct yhb_gpio_key *yhb_gpio_keys;

static irqreturn_t yhb_key_thread(int irq, void *d)
{
    struct yhb_gpio_key *keys = (struct yhb_gpio_key *)d;
    printk( "key %d val is %d\n", keys->gpio, gpio_get_value( keys->gpio ) );

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
            printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
        yhb_gpio_keys[i].gpio = gpio;
        yhb_gpio_keys[i].flags = flags;

        irq = gpio_to_irq( gpio );
        if( request_threaded_irq( irq, yhb_key_thread, NULL,
        IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "yhb_keys", &yhb_gpio_keys[i] ) )
        {
            printk( "ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__ );
            return -1;
        }
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