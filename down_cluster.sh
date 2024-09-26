#!/bin/bash

docker compose -f ./docker/docker-compose.yaml down && \
yes | docker image prune -a