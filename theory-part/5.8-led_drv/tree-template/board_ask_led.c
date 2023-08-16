
#include "board_ask_led_opt.h"

/* 
 * bus -> struct platform_device
*/
static struct resource _100ask_leds[] = 
{
    {
        .name = "5$3",
        .start = 0x020ac000, /* gpio5_3 */
        .end = 0x020affff, 
        .flags = IORESOURCE_MEM
    },
    {
        .name = "4$0",
        .start = 0x020a8000,/* gpio4_0*/
        .end = 0x020abfff,
        .flags = IORESOURCE_MEM
    }
};

static void empty_release(struct device *dev)
{
     return ;
}

static struct platform_device boardLeds = 
{
     .name = "100ask_led",
     .id_auto = 1,
     .num_resources =  sizeof(_100ask_leds) / sizeof(_100ask_leds[0]),
     .resource = _100ask_leds,
     .dev = {
          .release = empty_release
     }
};

static int __init board_ask_led_init(void)
{
    int error;

    error = platform_device_register(&boardLeds);
    if (error)
    {
        printk("ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return error;
    }

    return 0;
}

static void __exit board_ask_led_exit(void)
{
    return platform_device_unregister(&boardLeds);
}

module_init(board_ask_led_init);
module_exit(board_ask_led_exit);
MODULE_LICENSE("GPL");
