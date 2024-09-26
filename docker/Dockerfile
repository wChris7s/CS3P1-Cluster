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

# SETUP
COPY scripts/load_hosts.sh .
RUN  chmod +x ./load_hosts.sh

CMD ["/usr/sbin/sshd", "-D"]