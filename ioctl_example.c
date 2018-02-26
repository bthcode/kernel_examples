#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
 
#include "ioctl_example.h"
 
#define FIRST_MINOR 0
#define MINOR_CNT 1
 
static dev_t dev;
static struct cdev c_dev;
static struct class *cl;
static int status = 1, dignity = 3, ego = 5;

static bool timer_running = false;
static bool keep_timer_going = false;
static struct timer_list ioctl_exampletimer;
 
void ioctl_exampletimer_callback( unsigned long data )
{
    printk( "ioctl_exampletimer_callback called (%ld).\n", jiffies );

    if (keep_timer_going)
    {
        setup_timer( &ioctl_exampletimer, ioctl_exampletimer_callback, 0 );
        if (mod_timer( &ioctl_exampletimer, jiffies + msecs_to_jiffies(200) ))
        {
            printk("Error in timer callback\n");
            timer_running = false;
        }
        else
            timer_running = true;
    }
    else 
    {
        printk ("ioctl_exampletimer_callback, not starting new timer\n");
        timer_running = false;
    }
}


static int ioctl_exampleopen(struct inode *i, struct file *f)
{
    printk("IOCTL EXAMPLE OPENED\n");
    return 0;
}

static int ioctl_exampleclose(struct inode *i, struct file *f)
{
    int ret;
    printk("IOCTL EXAMPLE CLOSED\n");
    if (timer_running)
    {
      ret = del_timer( &ioctl_exampletimer );
      if (ret) printk("The timer is still in use...\n");
    }
    return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
static int ioctl_exampleioctl(struct inode *i, struct file *f, unsigned int cmd, unsigned long arg)
#else
static long ioctl_exampleioctl(struct file *f, unsigned int cmd, unsigned long arg)
#endif
{
    ioctl_example_arg_t q;
    int ctr;
    int * int_buf;
    ioctl_example_buf_t * dest_qb;

 
    switch (cmd)
    {
        case IOCTL_EXAMPLE_GET_VARIABLES:
            printk("IOCTL_EXAMPLE_GET_VARIABLES\n");
            q.status = status;
            q.dignity = dignity;
            q.ego = ego;
            if (copy_to_user((ioctl_example_arg_t *)arg, &q, sizeof(ioctl_example_arg_t)))
            {
                return -EACCES;
            }
            break;
        case IOCTL_EXAMPLE_CLR_VARIABLES:
            printk("IOCTL_EXAMPLE_CLR_VARIABLES\n");
            status = 0;
            dignity = 0;
            ego = 0;
            break;

        //--------------------------------------------------
        // THIS IOCTL ACCESSES USER MEMORY DIRECTLY
        //--------------------------------------------------
        case IOCTL_EXAMPLE_FILL_BUFFER:
            printk("IOCTL_EXAMPLE_FILL_BUFFER\n");
            // STRUCT WITH TARGET POINTER AND SIZE
            dest_qb = (ioctl_example_buf_t *) arg; 
            // CHECK MEMORY LOCATION
            if (!access_ok(VERIFY_WRITE, dest_qb->buf, dest_qb->len))
            {
                printk("ILLEGAL WRITE IN ioctl_exampleioctl\n");
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
	    case IOCTL_EXAMPLE_MEMCPY_BUFFER:
            printk("IOCTL_EXAMPLE_MEMCPY_BUFFER\n");
            // STRUCT WITH TARGET POINTER AND AIZE
            dest_qb = (ioctl_example_buf_t *) arg; 
            // CHECK MEMORY LOCATION
            if (!access_ok(VERIFY_WRITE, dest_qb->buf, dest_qb->len))
            {
                printk("ILLEGAL WRITE IN ioctl_exampleioctl\n");
                return -EACCES;
            }
            int_buf = (int *) dest_qb->buf;

            for (ctr=0; ctr<(dest_qb->len)/4; ctr ++)
            {
                put_user(ctr, int_buf + ctr);
            }
            
            break;
        case IOCTL_EXAMPLE_SET_VARIABLES:
            printk("IOCTL_EXAMPLE_SET_VARIABLES\n");
            if (copy_from_user(&q, (ioctl_example_arg_t *)arg, sizeof(ioctl_example_arg_t)))
            {
                return -EACCES;
            }
            status = q.status;
            dignity = q.dignity;
            ego = q.ego;
            break;
        case IOCTL_EXAMPLE_WORK_QUEUE:
            printk("IOCTL_EXAMPLE_TASK_QUEUE\n");
            break;
        case IOCTL_EXAMPLE_TASKLET:
            printk("IOCTL_EXAMPLE_TASKLET\n");
            break;
        case IOCTL_EXAMPLE_START_TIMER:
            printk("IOCTL_EXAMPLE_START_TIMER\n");
            keep_timer_going = true;
            setup_timer( &ioctl_exampletimer, ioctl_exampletimer_callback, 0 );
            if (mod_timer( &ioctl_exampletimer, jiffies + msecs_to_jiffies(200) ))
            {
                printk("Error in timer callback\n");
                timer_running = false;
            }
            else
                timer_running = true;
            break;
        default:
            return -EINVAL;
    }
 
    return 0;
}
 
static struct file_operations ioctl_example_fops =
{
    .owner = THIS_MODULE,
    .open = ioctl_exampleopen,
    .release = ioctl_exampleclose,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
    .ioctl = ioctl_exampleioctl
#else
    .unlocked_ioctl = ioctl_exampleioctl
#endif
};
 
static int __init ioctl_example_ioctl_init(void)
{
    int ret;
    struct device *dev_ret;
 

    printk("LOADING IOCTL EXAMPLE\n");
 
    if ((ret = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, "ioctl_example_ioctl")) < 0)
    {
        return ret;
    }
 
    cdev_init(&c_dev, &ioctl_example_fops);
 
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
 
static void __exit ioctl_example_ioctl_exit(void)
{
    printk("UNLOADING IOCTL EXAMPLE\n");
    device_destroy(cl, dev);
    class_destroy(cl);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, MINOR_CNT);
}
 
module_init(ioctl_example_ioctl_init);
module_exit(ioctl_example_ioctl_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anil Kumar Pugalia <email_at_sarika-pugs_dot_com>");
MODULE_DESCRIPTION("Query ioctl() Char Driver");
