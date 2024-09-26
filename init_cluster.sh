#!/bin/bash

if [ -z "$1" ] || [ -z "$2" ]; then
    echo "[ ERR ]: You must enter the following arguments: .\init_cluster <number of workers including the master> <default processes that each node will have>"
    exit 1
fi

rm -rf ./ssh && mkdir ./ssh
ssh-keygen -t rsa -q -N "" -f ./ssh/id_rsa
cp -r ./ssh ./docker

./docker/scripts/build_dc.sh "$1" "$2"
docker compose -f ./docker/docker-compose.yaml up -d --build

rm -rf ./docker/ssh
docker ps -a
docker exec -it master /bin/bash