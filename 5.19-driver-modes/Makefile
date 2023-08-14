KERN_DIR = /home/book/100ask_imx6ull-sdk/Linux-4.9.88

all:
	make -C $(KERN_DIR) M=`pwd` modules
	$(CROSS_COMPILE)gcc -o key_test key_test.c
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f led_drv_test

obj += block_key_drv.o
obj += poll_key_drv.o
obj += fasy_key_drv.o
obj += tasklet_key_drv.o
obj += timer_key_drv.o
