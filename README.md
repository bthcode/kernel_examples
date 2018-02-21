This repo contains a bunch of kernel / device driver examples that I have mostly borrowed from others.

My goal is to create a set of starting point examples that show all the major operations one might want to do.

HELLO: 
    This modules is the classic hello world example

    hello.c

    

IOCTL EXAMPLE:
    This module shows different IOCTLs, including setting kernel parameters, and moving buffers back and forth.

    query_app.c
    query_ioctl.h
    query_ioctl.c



SYSFS EXAMPLE:
    Example module for reading/writing sysfs files

    sysfs_example.c

