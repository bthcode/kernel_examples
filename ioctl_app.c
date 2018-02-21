#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
 
#include "ioctl_example.h"
 
void get_vars(int fd)
{
    ioctl_example_arg_t q;
 
    if (ioctl(fd, IOCTL_EXAMPLE_GET_VARIABLES, &q) == -1)
    {
        perror("ioctl_example_apps ioctl get");
    }
    else
    {
        printf("Status : %d\n", q.status);
        printf("Dignity: %d\n", q.dignity);
        printf("Ego    : %d\n", q.ego);
    }
}
void clr_vars(int fd)
{
    if (ioctl(fd, IOCTL_EXAMPLE_CLR_VARIABLES) == -1)
    {
        perror("ioctl_example_apps ioctl clr");
    }
}
void fill_buf(int fd)
{
    ioctl_example_buf_t qb;
    posix_memalign((void **) (&qb.buf), 4096, 4096);
    qb.len = 4096;
    if (ioctl(fd, IOCTL_EXAMPLE_FILL_BUFFER, &qb) == -1)
        perror("ioctl_example_apps ioctl fill buffer");
    int ii = 0;
    for (ii=0; ii<12; ii++)
        printf("%x ", (uint8_t) qb.buf[ii]); 
    printf("\n");
    for (ii=4086; ii<4096; ii++)
        printf("%x ", (uint8_t) qb.buf[ii]); 
    printf("\n");
   
}
void copy_buf(int fd)
{
    ioctl_example_buf_t qb;
    posix_memalign((void **) (&qb.buf), 4096, 4096);
    qb.len = 4096;
    if (ioctl(fd, IOCTL_EXAMPLE_MEMCPY_BUFFER, &qb) == -1)
        perror("ioctl_example_apps ioctl fill buffer");
    int ii = 0;
    for (ii=0; ii<12; ii++)
        printf("%x ", (uint8_t) qb.buf[ii]); 
    printf("\n");
    for (ii=4086; ii<4096; ii++)
        printf("%x ", (uint8_t) qb.buf[ii]); 

    printf("\n");
}

void start_timer(int fd)
{
 
    if (ioctl(fd, IOCTL_EXAMPLE_START_TIMER) == -1)
    {
        perror("ioctl_example_apps ioctl set");
    }
    sleep(5);
}

void set_vars(int fd)
{
    int v;
    ioctl_example_arg_t q;
 
    printf("Enter Status: ");
    scanf("%d", &v);
    getchar();
    q.status = v;
    printf("Enter Dignity: ");
    scanf("%d", &v);
    getchar();
    q.dignity = v;
    printf("Enter Ego: ");
    scanf("%d", &v);
    getchar();
    q.ego = v;
 
    if (ioctl(fd, IOCTL_EXAMPLE_SET_VARIABLES, &q) == -1)
    {
        perror("ioctl_example_apps ioctl set");
    }
}
 
int main(int argc, char *argv[])
{
    char *file_name = "/dev/query";
    int fd;
    enum
    {
        e_get,
        e_clr,
        e_set,
        e_buf,
	    e_cp,
        e_tim
    } option;
 
    if (argc == 1)
    {
        option = e_get;
    }
    else if (argc == 2)
    {
        if (strcmp(argv[1], "-g") == 0)
        {
            option = e_get;
        }
        else if (strcmp(argv[1], "-c") == 0)
        {
            option = e_clr;
        }
        else if (strcmp(argv[1], "-s") == 0)
        {
            option = e_set;
        }
        else if (strcmp(argv[1], "-b") == 0)
        {
            option = e_buf;
        }
        else if (strcmp(argv[1], "-p") == 0)
        {
            option = e_cp;
        }
        else if (strcmp(argv[1], "-t") == 0)
        {
            option = e_tim;
        }
        else
        {
            fprintf(stderr, "Usage: %s [-g | -c | -s | -b | -p | -t]\n", argv[0]);
            return 1;
        }
    }
    else
    {
        fprintf(stderr, "Usage: %s [-g | -c | -s | -p | -t]\n", argv[0]);
        return 1;
    }
    fd = open(file_name, O_RDWR);
    if (fd == -1)
    {
        perror("ioctl_example_apps open");
        return 2;
    }
 
    switch (option)
    {
        case e_get:
            get_vars(fd);
            break;
        case e_clr:
            clr_vars(fd);
            break;
        case e_set:
            set_vars(fd);
            break;
        case e_buf:
            fill_buf(fd);
            break;
        case e_cp:
            copy_buf(fd);
            break;
        case e_tim:
            start_timer(fd);
            break;
        default:
            break;
    }
 
    close (fd);
 
    return 0;
}
