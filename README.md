Here's the translation to English:

---

# Configuring an MPI Cluster using Docker

This project demonstrates how to set up a cluster using Docker and Docker Compose to run parallel programs with MPI (Message Passing Interface).

## Motivation

The goal of this project is to simplify the creation of an MPI cluster using Docker containers. This approach allows for an easy simulation of a distributed environment without the need to configure multiple physical or virtual machines.

## Requirements

- Linux operating system (Ubuntu distribution was used for testing purposes)
- [Docker](https://docs.docker.com/get-docker/)
- [Docker Compose](https://docs.docker.com/compose/install/)
- C/C++ and MPI (only if tests are needed outside the cluster)

## How it Works

### Dockerfile

The `Dockerfile` installs the necessary dependencies to run MPI and enables communication between nodes. Additionally, it includes a script that dynamically generates the hosts file to establish the connection between the master node and the workers.

### Docker Compose

The `docker-compose.yaml` file manages the creation of a network with containers, assigning hostnames, static IPs, and corresponding ports to each. It also simulates a shared volume that represents common storage between the nodes.

## Build, Run, and Shutdown

### Step 1: Start the Cluster

To deploy the cluster, run the following script from your local terminal:

```bash
./init_cluster.sh <number_of_containers> <processes_per_container>
```

- `<number_of_containers>`: Total number of containers (including the master node and workers).
- `<processes_per_container>`: Number of processes to run on each container.

After executing this command, a terminal connected to the master node will open.

### Step 2: Compile MPI Code

Once connected to the master node, you can compile your MPI code using the following command:

```bash
mpic++ -g -Wall -o ./shared/<executable> ./shared/<file_to_compile>
```

This will compile the specified C++ file and generate the executable inside the shared folder (`shared`), which is
accessible by all nodes in the cluster.

### Step 3: Run the Program

To run the program across the cluster, use the `mpiexec` command from the master node's terminal:

```bash
mpiexec --allow-run-as-root --hostfile hosts ./shared/<executable>
```

This will distribute the program across the containers defined in the `hosts` file, executing it on the cluster nodes.

### Step 4: Shut Down the Cluster

To stop the cluster and shut down all the nodes:

1. Exit the master node by running:
   ```bash
   exit
   ```
2. In your local terminal, run the following script to stop and remove the containers:
   ```bash
   ./down_cluster.sh
   ```