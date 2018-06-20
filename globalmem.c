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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zhijun Chen");
MODULE_DESCRIPTION("globel memory device driver");
MODULE_VERSION("0.1");

/* defines */
#define DEVICE_NAME  "global_memory"
#define MEM_SIZE     0x1000
#define GLOBALMEM_MAGIC 'g'
#define DEVICE_NUM      2
#define CLASS_NAME   "globalmem"

/* ioctl commands */
#define MEM_CLEAR _IO(GLOBALMEM_MAGIC, 0)

/* Prototypes */
int __init add_module(void);
void __exit remove_module(void);
static int globalmem_open( struct inode* inode, struct file* file);
static int globalmem_release( struct inode* inode, struct file* file);
static long globalmem_ioctl( struct file* file, unsigned int cmd, unsigned long arg);
static ssize_t globalmem_read( struct file* file, char __user* buf, size_t size, loff_t *ppos);
static ssize_t globalmem_write( struct file* file, const char __user* buf, size_t size, loff_t *ppos);

/* structures */
static struct file_operations globalmem_fops = {
    .open = globalmem_open,
    .unlocked_ioctl = globalmem_ioctl,
    .release = globalmem_release,
    .write = globalmem_write,
    .read = globalmem_read,
};

struct globalmem_dev {
    struct  cdev     cdev;
    unsigned char mem[MEM_SIZE];
    struct mutex mutex;
};

/* static variables*/
static int globalmem_major = 230;
static struct globalmem_dev* globalmem_devp;
static struct class *globalmem_class = NULL;
static void globalmem_setup_cdev(struct globalmem_dev* devp, int minor);

static void globalmem_setup_cdev(struct globalmem_dev* devp, int minor)
{
    int ret = 0;

    dev_t devno = MKDEV(globalmem_major, minor);

    mutex_init(&devp->mutex);
    cdev_init(&devp->cdev, &globalmem_fops);
    devp->cdev.owner = THIS_MODULE;

    ret = cdev_add(&devp->cdev, devno, 1);
    if (ret)
        printk(KERN_NOTICE "Error %d adding mem", ret);

    /* create device node */
    device_create(globalmem_class, NULL, devno, NULL, "globalmem%d", minor);
    printk(KERN_ERR "device create %s%d\n", DEVICE_NAME, minor);
    return;
}

/* functions */
int __init add_module(void)
{
    int ret = 0;
    int i = 0;

    /* register device number */
    dev_t devno = MKDEV(globalmem_major, 0);
    
    if (globalmem_major)
        ret = register_chrdev_region(devno, DEVICE_NUM,  DEVICE_NAME);
    else {
        ret = alloc_chrdev_region(&devno, 0, DEVICE_NUM, DEVICE_NAME);
    }
    
    printk(KERN_INFO "get device num!");

    /* set up cdev */
    if (0 == ret)
    {
        globalmem_devp = kzalloc(sizeof(struct globalmem_dev) * DEVICE_NUM, GFP_KERNEL);
        if (!globalmem_devp){
            ret = -ENOMEM;
        }  

    
        /* register class */
        globalmem_class = class_create(THIS_MODULE, CLASS_NAME);

	for (i = 0; i < DEVICE_NUM; i++)
	{
	    globalmem_setup_cdev(globalmem_devp + i, i);
            printk(KERN_INFO "setup cdev %d", ret);
	}
        
    }
    

    return 0;
}

void __exit remove_module(void)
{
    int i = 0;
    dev_t devno = MKDEV(globalmem_major, 0);

    for (i = 0; i < DEVICE_NUM; i++)
    {
        cdev_del(&(globalmem_devp + i)->cdev);

	device_destroy(globalmem_class, MKDEV(globalmem_major, i));
    }

    mutex_destroy(&(globalmem_devp + i)->mutex);

    kfree(globalmem_devp);

    printk(KERN_INFO "release module!\n");

    unregister_chrdev_region(devno, DEVICE_NUM);
    
    class_destroy(globalmem_class);

    return;
}

static int globalmem_open( struct inode* inode, struct file* file)
{
    file->private_data = globalmem_devp;
    return 0;
}

static int globalmem_release( struct inode* inode, struct file* file)
{
    return 0;
}

static ssize_t globalmem_write( struct file* file, const char __user* buf, size_t size, loff_t *ppos)
{
    unsigned long p = *ppos; 
    unsigned int count = size;
    int ret = 0;
    struct globalmem_dev* dev = file->private_data;

    if (p > MEM_SIZE)
    {
        return 0;
    }

    mutex_lock(&dev->mutex);

    if (count > MEM_SIZE - p)
        count = MEM_SIZE - p;

    if (copy_from_user(dev->mem + p, buf, count)){
        ret = -EFAULT;
    } else {
        *ppos += count;
	ret = count;
	printk(KERN_INFO "write %u bytes from %lu\n", count, p);
    }

    mutex_unlock(&dev->mutex);

    return ret;
}

static ssize_t globalmem_read( struct file* file, char __user* buf, size_t size, loff_t *ppos)
{
    unsigned long p = *ppos;
    unsigned int count = size;
    int ret = 0;
    struct globalmem_dev* dev = file->private_data;

    if (p > MEM_SIZE)
    {
        return 0;
    }

    if (count > MEM_SIZE - p)
        count = MEM_SIZE - p;

    mutex_lock(&dev->mutex);

    if (copy_to_user(buf, dev->mem + p, count)){
        ret = -EFAULT;
    } else {
        *ppos += count;
	ret = count;
	printk(KERN_INFO "read %u bytes from %lu\n", count, p);
    }

    mutex_unlock(&dev->mutex);

    return ret;
}

static long globalmem_ioctl( struct file* file, unsigned int cmd, unsigned long arg)
{
    struct globalmem_dev* dev = file->private_data;

    switch (cmd) {
    case MEM_CLEAR:
        memset(dev->mem, 0, MEM_SIZE);
	printk(KERN_INFO "globalmem is set to zero\n");
	break;
    default:
	return -EINVAL;
    }

    return 0;
}

module_init(add_module)
module_exit(remove_module)
