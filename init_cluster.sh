rm -rf ./ssh && mkdir ./ssh
ssh-keygen -t rsa -q -N "" -f ./ssh/id_rsa
cp -r ./ssh ./docker
cp -r ./source ./docker
docker compose -f ./docker/docker-compose.yaml up -d --build
rm -rf ./docker/ssh
rm -rf ./docker/source
docker ps -a
docker exec -it master /bin/bash