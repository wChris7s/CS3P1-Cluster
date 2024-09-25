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
RUN  mkdir /var/run/sshd
COPY ssh /root/.ssh/
COPY ssh/id_rsa.pub /root/.ssh/authorized_keys
RUN  chmod 700 /root/.ssh && chmod 600 /root/.ssh/authorized_keys

WORKDIR /home

# CPP
COPY source/mpi_mat_vect_mult.cpp .
COPY source/mpi_odd_even.cpp .
RUN  mpic++ -g -Wall -o mpi_mat_vect_mult mpi_mat_vect_mult.cpp
RUN  mpic++ -g -Wall -o mpi_odd_even mpi_odd_even.cpp

# DEBUG
RUN  mpic++ -DDEBUG -g -Wall -o  d_mpi_mat_vect_mult mpi_mat_vect_mult.cpp
RUN  mpic++ -DDEBUG -g -Wall -o  d_mpi_odd_even mpi_odd_even.cpp

# SETUP
COPY configure_mpi_hosts.sh .
RUN  chmod +x ./configure_mpi_hosts.sh

CMD ["/usr/sbin/sshd", "-D"]