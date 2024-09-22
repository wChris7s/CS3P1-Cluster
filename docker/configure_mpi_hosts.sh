#!/bin/bash

slots_size=1

touch hosts && \
echo "master slots=$slots_size"  >> hosts && \
echo "worker1 slots=$slots_size" >> hosts && \
echo "worker2 slots=$slots_size" >> hosts && \
echo "worker3 slots=$slots_size" >> hosts

/usr/sbin/sshd -D