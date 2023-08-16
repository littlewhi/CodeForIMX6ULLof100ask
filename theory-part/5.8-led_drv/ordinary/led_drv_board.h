
#define NUM_LED 1
#ifndef LED_DRV_BOARD_H
#define LED_DRV_BOARD_H

struct led_operations
{
    int num; /* led数量 */
    int (*write)(char); /* write */
    int (*read)(char *, unsigned int); /* read */
    int (*open)(short, short); /* open */
    int (*close)(void); /* close */
};

struct led_operations * get_led_operations(void);

#endif