ifneq ($(KERNELRELEASE),)
obj-m := ihs_gpio.o
else
PWD := $(shell pwd)
#KVER ?= $(shell uname -r)
KVER ?= linux-3.0.35
KDIR := /tftpboot/freescale/linux-3.0.35-sgtl5000
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	arm-linux-gcc ihs_gpio_test.c -o ihs_gpio_test
	cp ihs_gpio.ko /tftpboot/freescale/rootfs/lib/modules/
	cp ihs_gpio_test /tftpboot/freescale/rootfs/root
	
clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions *.symvers *.order
endif

