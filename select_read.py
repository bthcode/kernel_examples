#!/usr/bin/env python

fin = open('/sys/kernel/debug/select_example', 'rb')

import time
import numpy as np
import select

buf = ''
while 1:
    inputs  = [fin]
    outputs = []
    readable, writable, exceptional = select.select(inputs, outputs, inputs, 0.5)
    if fin in readable:
        buf += fin.read()
        buflen = np.floor( len(buf) / 4 ) * 4
        buflen = int(buflen)
        x = np.frombuffer(buf[:buflen], dtype=np.uint32)

        if buflen < len(buf):
            buf = buf[buflen:]
        else:
            buf = ''
        print np.diff(x)
    else:
        print "NOPE"

fin.close()
