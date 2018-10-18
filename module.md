加载 - __init fn()  
rmmod - __exit fn()

/proc/kallsyms  
EXPORT_SYMBOL()  
EXPORT_SYMBOL_GPL()  

Makefile
```cpp
KVERS = $(shell uname - r)

#kernel module
obj-m += hello.o

EXTRA_CFLAGS = -g - O0

build: all

all:
  make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules
    
clean:
  make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean
```
