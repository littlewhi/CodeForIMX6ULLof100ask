
#include "button_opt.h"
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/slab.h>


static unsigned int *minorTopin;

static int yhb_button_probe(struct platform_device *dev)
{
    int gp, i;
    if(dev->dev.of_node == NULL)
        return -1;

    if(of_property_read_u32(dev->dev.of_node, "pin", &gp))
    {
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    for(i = 0; i < MLEN; ++i)
    {
        if(minorTopin[i] == -1)
        {
            minorTopin[i] = gp;
            return button_device_create(i);
        }
    }

    printk("There is no space for a new button device.\n");
    return -1;
}

static int yhb_button_remove(struct platform_device *dev)
{
    int gp, i;
    if(dev->dev.of_node == NULL)
        return -1;

    if(of_property_read_u32(dev->dev.of_node, "pin", &gp))
    {
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    for(i = 0; i < MLEN; ++i)
    {
        if(minorTopin[i] == gp)
        {
            minorTopin[i] = -1;
            button_device_destroy(i);
            return 0;
        }
    }

    printk("Not find the device exists.\n");
    return -1;
}

static const struct of_device_id yhb_button_id[] = {
	{ .compatible = "yhb_button" },
	{ /* sentinel */ }
};

struct platform_driver platform_button = 
{
    .probe = yhb_button_probe,
    .remove = yhb_button_remove,
    .driver = {
		.name = "yhb_button",
		.of_match_table = yhb_button_id,
	}
};

static int __init yhb_button_init(void)
{
    int i;
    minorTopin = get_minor_to_pin();
    for(i = 0; i< MLEN; ++i)
    {
        minorTopin[i] = -1;
    }

    get_button_operations_pointer(get_button_operations_pointer_from_opt());

    if(platform_driver_register(&platform_button))
    {
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -ENODEV;
    }

    return 0;
}

static void __exit yhb_button_exit(void)
{
    return platform_driver_unregister(&platform_button);
}



module_init(yhb_button_init);
module_exit(yhb_button_exit);

MODULE_LICENSE("GPL");