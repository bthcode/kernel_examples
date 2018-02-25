#include <asm/uaccess.h>
#include <linux/circ_buf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/mm.h>  
#include <linux/poll.h>
 
#ifndef VM_RESERVED
# define  VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)
#endif
 
struct dentry  *file;
struct dentry  *file2;

struct select_example_circ_buffer {
    char * buf;
    unsigned long head;
    unsigned long tail;
    unsigned long size;
};
 
struct select_example_info
{
    char *data;            
    int reference;      
};

static struct select_example_circ_buffer se_buf;
static struct spinlock producer_lock;
static DECLARE_WAIT_QUEUE_HEAD(select_example_wait);
static DECLARE_WAIT_QUEUE_HEAD(select_example_input_wait);
//------------------------------------------------------------------
// OUTPUT FILE HANDLE
//------------------------------------------------------------------

static unsigned int select_example_poll(struct file *filp, poll_table *wait)
{
    int available;
    unsigned long head;
    unsigned long tail;
    printk("SELECT EXAMPLE SELECT\n"); 
    poll_wait(filp, &select_example_wait, wait);

    spin_lock(&producer_lock);

    head = ACCESS_ONCE(se_buf.head);
    tail = ACCESS_ONCE(se_buf.tail);

    available = CIRC_CNT(head, tail, se_buf.size);
    if (available > 0)
    {
        printk("...True\n");
        spin_unlock(&producer_lock);
        return POLLIN | POLLRDNORM;
    }
    else
    {
        printk("...Nope\n");
        spin_unlock(&producer_lock);
        return 0;
    }
}; // END select_example_poll

int select_example_close(struct inode *inode, struct file *filp)
{
    printk("SELECT EXAMPLE CLOSE\n"); 
    return 0;
}

 
int select_example_open(struct inode *inode, struct file *filp)
{
    printk("SELECT EXAMPLE OPEN\n"); 
    return 0;
}


static ssize_t select_example_read(struct file *filp, char *buffer,  size_t length, loff_t *offset)
{
    int available;
    int idx;
    char val;
    unsigned long head;

    spin_lock(&producer_lock);

    head = ACCESS_ONCE(se_buf.head);

    available = CIRC_CNT(head, se_buf.tail, se_buf.size);
    if (available < 1)
    {
        spin_unlock(&producer_lock);
        return 0;
    }
    else
    {
        if (available > length)
            available = length;

        //printk("SELECT_READ: %d\n", available);

        for (idx=0; idx<available; idx++)
        {
            val = se_buf.buf[se_buf.tail];
            put_user(val, buffer++);
            se_buf.tail = (se_buf.tail + 1) & (se_buf.size - 1);
        }
        spin_unlock(&producer_lock);
        return available;
    }
} // END - select example read



static const struct file_operations select_example_fops = {
    .open = select_example_open,
    .release = select_example_close,
    .poll = select_example_poll,
    .read = select_example_read
};



//------------------------------------------------------------------
// INPUT FILE HANDLE
//------------------------------------------------------------------
int select_example_input_open(struct inode *inode, struct file *filp)
{
    printk("SELECT EXAMPLE INPUT OPEN\n");
    return 0;
}

int select_example_input_close(struct inode *inode, struct file *filp)
{
    printk("SELECT EXAMPLE INPUT CLOSE\n");
    return 0;
}

static unsigned int select_example_input_poll(struct file *filp, poll_table *wait)
{
    int space;
    unsigned long head;
    unsigned long tail;
    printk("SELECT EXAMPLE INPUT SELECT\n"); 
    poll_wait(filp, &select_example_input_wait, wait);

    spin_lock(&producer_lock);
    head = ACCESS_ONCE(se_buf.head);
    tail = ACCESS_ONCE(se_buf.tail);

    space = CIRC_SPACE(head, tail, se_buf.size);
    if (space > 0)
    {
        printk("...True\n");
        spin_unlock(&producer_lock);
        return POLLWRNORM;
    }
    else
    {
        printk("...Nope\n");
        spin_unlock(&producer_lock);
        return 0;
    }
}; // END select_example_input_poll


ssize_t select_example_input_write (struct file * filep , const char * buf, size_t length, loff_t * offset)
{
    unsigned long tail;
    int space;
    int idx;
    char tmp;

    spin_lock(&producer_lock);

    tail = READ_ONCE(se_buf.tail);

    space = CIRC_SPACE(se_buf.head, tail, se_buf.size);

    if (space >= 1)
    {
        if (space > length)
            space = length;

        printk("SELECT writing %u\n", space);

        for (idx=0; idx<space; idx++)
        {
            get_user(tmp, buf + idx);
            //if (idx % 100 == 0)
            //    printk("WRITE=%d\n", tmp);
            se_buf.buf[se_buf.head] = tmp;
            se_buf.head  = (se_buf.head + 1) & (se_buf.size - 1);
        }
    }
    else
    {
    }

    spin_unlock(&producer_lock);
    return space;
} // END select_example_input_write


 

static const struct file_operations select_example_input_fops = {
    .open = select_example_input_open,
    .release = select_example_input_close,
    .write = select_example_input_write,
    .poll  = select_example_input_poll
};


 
static int __init select_example_module_init(void)
{
    printk("SELECT EXAMPLE INIT\n"); 
    file = debugfs_create_file("select_example", 0400, NULL, NULL, &select_example_fops);
    file2 = debugfs_create_file("select_example_input", 0200, NULL, NULL, &select_example_input_fops);
    se_buf.buf = (char*) kmalloc( PAGE_SIZE, GFP_KERNEL );
    se_buf.head = 0;
    se_buf.tail = 0;
    se_buf.size = PAGE_SIZE;
    spin_lock_init(&producer_lock);
    return 0;
};



 
static void __exit select_example_module_exit(void)
{
    printk("SELECT EXAMPLE EXIT\n"); 
    debugfs_remove(file);
    debugfs_remove(file2);
    kfree(se_buf.buf);
};


 
module_init(select_example_module_init);
module_exit(select_example_module_exit);
MODULE_LICENSE("GPL");
