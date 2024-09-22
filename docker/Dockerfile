FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y \
    build-essential \
    openssh-server \
    openmpi-bin \
    openmpi-common \
    openmpi-doc \
    libopenmpi-dev

RUN sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config
RUN echo "    StrictHostKeyChecking no" >> /etc/ssh/ssh_config
RUN echo "    UserKnownHostsFile /dev/null" >> /etc/ssh/ssh_config
RUN echo "    LogLevel ERROR" >> /etc/ssh/ssh_config

# SSH
RUN mkdir /var/run/sshd
COPY ssh /root/.ssh/
COPY ssh/id_rsa.pub /root/.ssh/authorized_keys
RUN  chmod 700 /root/.ssh && chmod 600 /root/.ssh/authorized_keys

WORKDIR /home

# CPP
COPY source/main.cpp .
RUN  mpic++ main.cpp -o main

# SETUP
COPY configure_mpi_hosts.sh .
RUN  chmod +x ./configure_mpi_hosts.sh

CMD ["/usr/sbin/sshd", "-D"]

# ssh-keygen -t rsa -q -N "" -f ./ssh/id_rsa
# mpirun --allow-run-as-root -hostfile hosts ./main
# mpirun --allow-run-as-root -H master,worker1,worker2,worker3 ./main
# mpirun --allow-run-as-root --hostfile hosts -np 4 ./main
# ssh <hostname>
# slots same as np, by default np is 4