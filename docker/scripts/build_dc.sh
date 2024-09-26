#!/bin/bash

NUM_WORKERS=$(expr "$1" - 1)
NUM_SLOTS=$2

if [ -z "$NUM_WORKERS" ]; then
    echo "You must enter the number of workers including the master for the creation of the cluster."
    exit 1
fi

cat > docker/docker-compose.yaml <<EOL
x-common-config: &common-config-template
  build:
    context: .
    dockerfile: Dockerfile
  extra_hosts:
      - "master:10.0.0.5"
EOL

for i in $(seq 1 "$NUM_WORKERS"); do
    WORKER_IP=$((5 + $i))
    echo "      - \"worker$i:10.0.0.$WORKER_IP\"" >> docker/docker-compose.yaml
done

cat >> docker/docker-compose.yaml <<EOL
  volumes:
      - ../source:/home/shared
services:
  master:
    <<: *common-config-template
    container_name: master
    hostname: master
    networks:
      cluster-network:
        ipv4_address: 10.0.0.5
    ports:
      - "9095:22"
    command: /bin/bash -c "./load_hosts.sh $NUM_WORKERS $NUM_SLOTS"
EOL

for i in $(seq 1 "$NUM_WORKERS"); do
    WORKER_IP=$((5 + $i))
    WORKER_PORT=$((9095 + $i))
    cat >> docker/docker-compose.yaml <<EOL
  worker$i:
    <<: *common-config-template
    container_name: worker$i
    hostname: worker$i
    networks:
      cluster-network:
        ipv4_address: 10.0.0.$WORKER_IP
    ports:
      - "$WORKER_PORT:22"
EOL
done

cat >> docker/docker-compose.yaml <<EOL
networks:
  cluster-network:
    driver: bridge
    ipam:
      config:
      - subnet: 10.0.0.0/24
EOL