obj-m = hello.o
EXTRA_CFLAGS += $(CFLAGS_EXTRA) -fno-pie

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=/home/zjchen/dev/kernel_driver/linux_driver modules

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=/home/zjchen/dev/kernel_driver/linux_driver clean

