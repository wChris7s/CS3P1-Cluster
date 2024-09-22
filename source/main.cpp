#include <mpi.h>
#include <stdio.h>
#include <iostream>

int main(int argc, char** argv) {

	std::string default_message = "@%s: Hello from process [ %i ] out of [ %i ] processes.\n";

	MPI_Init(&argc, &argv);
  
	int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
	MPI_Get_processor_name(processor_name, &name_len);

	if (world_rank == 0) {
		printf(default_message.c_str(), processor_name, world_rank, world_size);
	} else {
    printf(default_message.c_str(), processor_name, world_rank, world_size);
	}

  MPI_Finalize();
  return 0;
}
