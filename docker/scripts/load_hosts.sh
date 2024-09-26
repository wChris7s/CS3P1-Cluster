#!/bin/bash

NUM_WORKERS=$1
NUM_SLOTS=$2

if [ -z "$NUM_WORKERS" ]; then
    echo "You must enter the number of workers including the master for the creation of the cluster."
    exit 1
fi

# Clear hosts file
true > hosts

echo "master slots=$NUM_SLOTS" >> hosts
for i in $(seq 1 "$NUM_WORKERS"); do
    echo "worker$i slots=$NUM_SLOTS" >> hosts
done

/usr/sbin/sshd -D