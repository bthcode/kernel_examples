#ifndef QUERY_IOCTL_H
#define QUERY_IOCTL_H
#include <linux/ioctl.h>
 
typedef struct
{
    int status, dignity, ego;
} query_arg_t;

typedef struct
{
    char * buf;
    size_t len;
} query_buf_t;
 
#define QUERY_GET_VARIABLES _IOR('q', 1, query_arg_t *)
#define QUERY_CLR_VARIABLES _IO('q', 2)
#define QUERY_SET_VARIABLES _IOW('q', 3, query_arg_t *)
#define QUERY_FILL_BUFFER   _IOW('q', 4, query_buf_t *)
#define QUERY_MEMCPY_BUFFER _IOW('q', 5, query_buf_t *)
#define QUERY_WORK_QUEUE    _IO('q', 6)
#define QUERY_TASKLET       _IO('q', 7)
#define QUERY_START_TIMER   _IO('q', 8)
 
#endif
