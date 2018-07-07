obj-m = globalfifo.o
EXTRA_CFLAGS += $(CFLAGS_EXTRA) -fno-pie

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=/home/zjchen/dev/kernel_driver/global_fifo modules

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=/home/zjchen/dev/kernel_driver/global_fifo clean

