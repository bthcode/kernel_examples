#ifndef IOCTL_EXAMPLE_IOCTL_H
#define IOCTL_EXAMPLE_IOCTL_H
#include <linux/ioctl.h>
 
typedef struct
{
    int status, dignity, ego;
} ioctl_example_arg_t;

typedef struct
{
    char * buf;
    size_t len;
} ioctl_example_buf_t;
 
#define IOCTL_EXAMPLE_GET_VARIABLES _IOR('q', 1, ioctl_example_arg_t *)
#define IOCTL_EXAMPLE_CLR_VARIABLES _IO('q', 2)
#define IOCTL_EXAMPLE_SET_VARIABLES _IOW('q', 3, ioctl_example_arg_t *)
#define IOCTL_EXAMPLE_FILL_BUFFER   _IOW('q', 4, ioctl_example_buf_t *)
#define IOCTL_EXAMPLE_MEMCPY_BUFFER _IOW('q', 5, ioctl_example_buf_t *)
#define IOCTL_EXAMPLE_WORK_QUEUE    _IO('q', 6)
#define IOCTL_EXAMPLE_TASKLET       _IO('q', 7)
#define IOCTL_EXAMPLE_START_TIMER   _IO('q', 8)
 
#endif
