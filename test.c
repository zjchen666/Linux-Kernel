/* 
 * hello.c
 */
#include <linux/module.h>
#include <linux/init.h>

/* Prototypes */
int __init add_module(void);
void __exit remove_module(void);

/* static variables*/
static int major = 0;
#define DEVICE_NAME  "hello"

int __init add_module(void)
{
	printk(KERN_INFO "Hello World!\n");
	printk("The major number is: %d\n", major);
	return 0;
}

void __exit remove_module(void)
{
	printk(KERN_INFO "release module!\n");
	return;
}

module_init(add_module)
module_exit(remove_module)
