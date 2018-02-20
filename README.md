This repo contains a bunch of kernel / device driver examples that I have mostly borrowed from others.

My goal is to create a set of starting point examples that show all the major operations one might want to do.

HELLO:
    hello.c

    This modules is the classic hello world example

IOCTL EXAMPLE:
    query_app.c
    query_ioctl.h
    query_ioctl.c

    This module shows different IOCTLs, including setting kernel parameters, and moving buffers back and forth.


SYSFS EXAMPLE:
    sysfs_example.c

    Example module for reading/writing sysfs files
