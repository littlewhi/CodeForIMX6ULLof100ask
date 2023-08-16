KERN_DIR = /home/book/100ask_imx6ull-sdk/Linux-4.9.88

all:
	make -C $(KERN_DIR) M=`pwd` modules
	$(CROSS_COMPILE)gcc -o button_drv_test button_drv_test.c
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f button_drv_test

button_bus_opt-y := button_bus.o button_opt.o
obj-m += button_bus_opt.o button_drv.o
