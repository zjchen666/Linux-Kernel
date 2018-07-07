/* hello.c
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zhijun Chen");
MODULE_DESCRIPTION("globel fifo device driver");
MODULE_VERSION("0.1");

/* defines */
#define DEVICE_NAME  "global_fifo"
#define FIFO_SIZE     0x1000
#define GLOBALFIFO_MAGIC 'g'
#define DEVICE_NUM      2
#define CLASS_NAME   "globalfifo"

/* ioctl commands */
#define FIFO_CLEAR _IO(GLOBALFIFO_MAGIC, 0)

/* Prototypes */
int __init add_module(void);
void __exit remove_module(void);
static int globalfifo_open( struct inode* inode, struct file* file);
static int globalfifo_release( struct inode* inode, struct file* file);
static long globalfifo_ioctl( struct file* file, unsigned int cmd, unsigned long arg);
static ssize_t globalfifo_read( struct file* file, char __user* buf, size_t size, loff_t *ppos);
static ssize_t globalfifo_write( struct file* file, const char __user* buf, size_t size, loff_t *ppos);

/* structures */
static struct file_operations globalfifo_fops = {
    .open = globalfifo_open,
    .unlocked_ioctl = globalfifo_ioctl,
    .release = globalfifo_release,
    .write = globalfifo_write,
    .read = globalfifo_read,
};

struct globalfifo_dev {
    struct  cdev     cdev;
    unsigned char mem[FIFO_SIZE];
    struct mutex mutex;
    unsigned int current_len;
    wait_queue_head_t r_wait;
    wait_queue_head_t w_wait;
};

/* static variables*/
static int globalfifo_major = 0;
static struct globalfifo_dev* globalfifo_devp;
static struct class *globalfifo_class = NULL;
static void globalfifo_setup_cdev(struct globalfifo_dev* devp, int minor);

static void globalfifo_setup_cdev(struct globalfifo_dev* devp, int minor)
{
    int ret = 0;

    dev_t devno = MKDEV(globalfifo_major, minor);

    mutex_init(&devp->mutex);
    init_waitqueue_head(&devp->r_wait);
    init_waitqueue_head(&devp->w_wait);

    cdev_init(&devp->cdev, &globalfifo_fops);
    devp->cdev.owner = THIS_MODULE;

    ret = cdev_add(&devp->cdev, devno, 1);
    if (ret)
        printk(KERN_NOTICE "Error %d adding fifo", ret);

    /* create device node */
    device_create(globalfifo_class, NULL, devno, NULL, "globalfifo%d", minor);
    printk(KERN_ERR "device create %s%d\n", DEVICE_NAME, minor);
    return;
}

/* functions */
int __init add_module(void)
{
    int ret = 0;
    int i = 0;

    /* register device number */
    dev_t devno = MKDEV(globalfifo_major, 0);
    
    if (globalfifo_major)
        ret = register_chrdev_region(devno, DEVICE_NUM,  DEVICE_NAME);
    else {
        ret = alloc_chrdev_region(&devno, 0, DEVICE_NUM, DEVICE_NAME);
	globalfifo_major = MAJOR(devno);
    }
    
    printk(KERN_INFO "get device num!");

    /* set up cdev */
    if (0 == ret)
    {
        globalfifo_devp = kzalloc(sizeof(struct globalfifo_dev) * DEVICE_NUM, GFP_KERNEL);
        if (!globalfifo_devp){
            ret = -ENOMEM;
        }  

    
        /* register class */
        globalfifo_class = class_create(THIS_MODULE, CLASS_NAME);

	for (i = 0; i < DEVICE_NUM; i++)
	{
	    globalfifo_setup_cdev(globalfifo_devp + i, i);
            printk(KERN_INFO "setup cdev %d", ret);
	}
        
    }
    

    return 0;
}

void __exit remove_module(void)
{
    int i = 0;
    dev_t devno = MKDEV(globalfifo_major, 0);

    for (i = 0; i < DEVICE_NUM; i++)
    {
        cdev_del(&(globalfifo_devp + i)->cdev);

	device_destroy(globalfifo_class, MKDEV(globalfifo_major, i));
    }

    mutex_destroy(&(globalfifo_devp + i)->mutex);
    
    kfree(globalfifo_devp);

    printk(KERN_INFO "release module!\n");

    unregister_chrdev_region(devno, DEVICE_NUM);
    
    class_destroy(globalfifo_class);

    return;
}

static int globalfifo_open( struct inode* inode, struct file* file)
{
    file->private_data = globalfifo_devp;
    return 0;
}

static int globalfifo_release( struct inode* inode, struct file* file)
{
    return 0;
}

static ssize_t globalfifo_write( struct file* file, const char __user* buf, size_t size, loff_t *ppos)
{
    unsigned long p = *ppos; 
    unsigned int count = size;
    int ret = 0;
    struct globalfifo_dev* dev = file->private_data;

    DECLARE_WAITQUEUE(wait, current);


    if (p > FIFO_SIZE)
    {
        return 0;
    }

    mutex_lock(&dev->mutex);

    add_wait_queue(&dev->w_wait, &wait);

    while(dev->current_len == FIFO_SIZE)
    {
        if (file->f_flags & O_NONBLOCK)
	{
            ret = -EAGAIN;
	    goto out;
	}

	__set_current_state(TASK_INTERRUPTIBLE);
	mutex_unlock(&dev->mutex);

	schedule();

	if (signal_pending(current))
	{
	    ret = -ERESTARTSYS;
	    goto out2;
	}

	mutex_lock(&dev->mutex);
    }

    if (count > FIFO_SIZE - dev->current_len)
        count = FIFO_SIZE - dev->current_len;

    if (copy_from_user(dev->mem + dev->current_len, buf, count)){
        ret = -EFAULT;
	goto out;
    } else {
	dev->current_len += count;
	printk(KERN_INFO "write %u bytes from %lu\n", count, p);

	wake_up_interruptible(&dev->r_wait);

	ret = count;
    }

    out:
    mutex_unlock(&dev->mutex);

    out2:
    remove_wait_queue(&dev->w_wait, &wait);
    set_current_state(TASK_RUNNING);

    return ret;
}

static ssize_t globalfifo_read( struct file* file, char __user* buf, size_t size, loff_t *ppos)
{
    unsigned long p = *ppos;
    unsigned int count = size;
    int ret = 0;
    struct globalfifo_dev* dev = file->private_data;

    DECLARE_WAITQUEUE(wait, current);

    if (p > FIFO_SIZE)
    {
        return 0;
    }

    if (count > FIFO_SIZE - p)
        count = FIFO_SIZE - p;

    mutex_lock(&dev->mutex);

    add_wait_queue(&dev->r_wait, &wait);

    while (dev->current_len == 0)
    {
        if (file->f_flags & O_NONBLOCK)
	{
	    ret = -EAGAIN;
	    goto out;
	}

	__set_current_state(TASK_INTERRUPTIBLE);
	mutex_unlock(&dev->mutex);

	schedule();

	if (signal_pending(current))
	{
	    ret = -ERESTARTSYS;
	    goto out2;
	}
	mutex_lock(&dev->mutex);
    }

    if (count > dev->current_len)
    {
        count = dev->current_len;
    }

    if (copy_to_user(buf, dev->mem + p, count)){
        ret = -EFAULT;
	goto out;
    } else {
        *ppos += count;
        memcpy(dev->mem, dev->mem + count, dev->current_len - count);
	dev->current_len -= count;
	ret = count;
	wake_up_interruptible(&dev->w_wait);

	printk(KERN_INFO "read %u bytes from %lu\n", count, p);
    }

    out:
    mutex_unlock(&dev->mutex);

    out2:
    remove_wait_queue(&dev->r_wait, &wait);
    set_current_state(TASK_RUNNING);

    return ret;
}

static long globalfifo_ioctl( struct file* file, unsigned int cmd, unsigned long arg)
{
    struct globalfifo_dev* dev = file->private_data;

    switch (cmd) {
    case FIFO_CLEAR:
        memset(dev->mem, 0, FIFO_SIZE);
	printk(KERN_INFO "globalfifo is set to zero\n");
	break;
    default:
	return -EINVAL;
    }

    return 0;
}

module_init(add_module)
module_exit(remove_module)
