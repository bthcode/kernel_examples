#!/usr/bin/env python

import select
import struct
import time

fin = open('/sys/kernel/debug/select_example_input', 'wb')

inputs = []
outputs = [fin]

ctr = 0
while 1:
    readable, writable, exceptional = select.select(inputs, outputs, inputs, 1)
    if fin in writable:
        print "WRITING"
        s = ''
        for j in range(100):
            s += struct.pack('I', ctr)
            ctr+=1
        fin.write(s)
        fin.flush()
    else:
        print "NOPE"

fin.close()


