#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
 
#include "query_ioctl.h"
 
#define FIRST_MINOR 0
#define MINOR_CNT 1
 
static dev_t dev;
static struct cdev c_dev;
static struct class *cl;
static int status = 1, dignity = 3, ego = 5;
static int myval = 0;


static int my_open(struct inode *i, struct file *f)
{
    printk("IOCTL EXAMPLE OPENED\n");
    return 0;
}

static int my_close(struct inode *i, struct file *f)
{
    printk("IOCTL EXAMPLE CLOSED\n");
    return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
static int my_ioctl(struct inode *i, struct file *f, unsigned int cmd, unsigned long arg)
#else
static long my_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
#endif
{
    query_arg_t q;
    int ctr;
    char * localbuf;
    query_buf_t * dest_qb;

 
    switch (cmd)
    {
        case QUERY_GET_VARIABLES:
            printk("QUERY_GET_VARIABLES\n");
            q.status = status;
            q.dignity = dignity;
            q.ego = ego;
            if (copy_to_user((query_arg_t *)arg, &q, sizeof(query_arg_t)))
            {
                return -EACCES;
            }
            break;
        case QUERY_CLR_VARIABLES:
            printk("QUERY_CLR_VARIABLES\n");
            status = 0;
            dignity = 0;
            ego = 0;
            break;

        //--------------------------------------------------
        // THIS IOCTL ACCESSES USER MEMORY DIRECTLY
        //--------------------------------------------------
        case QUERY_FILL_BUFFER:
            printk("QUERY_FILL_BUFFER\n");
            // STRUCT WITH TARGET POINTER AND SIZE
            dest_qb = (query_buf_t *) arg; 
            // CHECK MEMORY LOCATION
            if (!access_ok(VERIFY_WRITE, dest_qb->buf, dest_qb->len))
            {
                printk("ILLEGAL WRITE IN my_ioctl\n");
                return -EACCES;
            }
            // WRITE TO THE MEMORY
            for (ctr=0; ctr<4096; ctr += 2)
	        {
                put_user(0xaa, dest_qb->buf + ctr);
                put_user(0x55, dest_qb->buf + ctr + 1);
	        }
	        break;

        //--------------------------------------------------
        // THIS IOCTL SHOWS A COPY TO USER SPACE
        //--------------------------------------------------
	    case QUERY_MEMCPY_BUFFER:
            printk("QUERY_MEMCPY_BUFFER\n");
            // STRUCT WITH TARGET POINTER AND AIZE
            dest_qb = (query_buf_t *) arg; 
            // CHECK MEMORY LOCATION
            if (!access_ok(VERIFY_WRITE, dest_qb->buf, dest_qb->len))
            {
                printk("ILLEGAL WRITE IN my_ioctl\n");
                return -EACCES;
            }

            // CREATE A TEST PATTERN - EACH READ WILL START FROM A NEW 'myval'
            localbuf = (char *) kmalloc(PAGE_SIZE, GFP_USER);
            for (ctr=0; ctr<dest_qb->len; ctr++) 
                localbuf[ctr] = myval + ctr % 256; 
            myval += 1;

            // COPY TO USER
            if (copy_to_user(dest_qb->buf, localbuf, dest_qb->len))
            {
                return -EACCES;
            }
            
            // FREE THE TEST PATTERN
            kfree(localbuf);
            break;
        case QUERY_SET_VARIABLES:
            printk("QUERY_SET_VARIABLES\n");
            if (copy_from_user(&q, (query_arg_t *)arg, sizeof(query_arg_t)))
            {
                return -EACCES;
            }
            status = q.status;
            dignity = q.dignity;
            ego = q.ego;
            break;
        default:
            return -EINVAL;
    }
 
    return 0;
}
 
static struct file_operations query_fops =
{
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
    .ioctl = my_ioctl
#else
    .unlocked_ioctl = my_ioctl
#endif
};
 
static int __init query_ioctl_init(void)
{
    int ret;
    struct device *dev_ret;
 

    printk("LOADING IOCTL EXAMPLE\n");
 
    if ((ret = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, "query_ioctl")) < 0)
    {
        return ret;
    }
 
    cdev_init(&c_dev, &query_fops);
 
    if ((ret = cdev_add(&c_dev, dev, MINOR_CNT)) < 0)
    {
        return ret;
    }
     
    if (IS_ERR(cl = class_create(THIS_MODULE, "char")))
    {
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(cl);
    }
    if (IS_ERR(dev_ret = device_create(cl, NULL, dev, NULL, "query")))
    {
        class_destroy(cl);
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(dev_ret);
    }
 
    return 0;
}
 
static void __exit query_ioctl_exit(void)
{
    printk("UNLOADING IOCTL EXAMPLE\n");
    device_destroy(cl, dev);
    class_destroy(cl);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, MINOR_CNT);
}
 
module_init(query_ioctl_init);
module_exit(query_ioctl_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anil Kumar Pugalia <email_at_sarika-pugs_dot_com>");
MODULE_DESCRIPTION("Query ioctl() Char Driver");
