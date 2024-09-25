#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <mpi.h>

void print_err(const std::string& fname, const std::string& message, MPI_Comm comm) {
  int my_rank;
  MPI_Comm_rank(comm, &my_rank);
  if (my_rank == 0) {
    std::cerr << "[ERR] - [ p." << my_rank << " ] - [ " << fname << " ] => " << message << std::endl;
  }
}

void hasErrors(int no_err_flag, const std::string& fname, const std::string& message, MPI_Comm comm) {
  int is_ok;
  /* Combina los valores del flag de todos los procesos y almacena el
   * resultado para luego encontrar el mínimo entre todos. */
  MPI_Allreduce(&no_err_flag, &is_ok, 1, MPI_INT, MPI_MIN, comm);
  if (is_ok == 0) {
    print_err(fname, message, comm);
    MPI_Finalize();
    exit(-1);
  }
}

void setDim(int* m_p, int* local_m_p, int* n_p, int* local_n_p, int my_rank, int comm_sz, MPI_Comm comm)
{
  int no_err_flag = 1;

  if (my_rank == 0) {
    std::cout << "Ingrese el número de filas:" << std::endl;
    std::cin >> *m_p;
    std::cout << "Ingrese el número de columnas:" << std::endl;
    std::cin >> *n_p;
  }

  /* Se envía/transmite las dimensiones a todos los procesos. */
  MPI_Bcast(m_p, 1, MPI_INT, 0, comm);
  MPI_Bcast(n_p, 1, MPI_INT, 0, comm);

  /* Sé verífica que las dimensiones sean válidas y divisibles por el número de procesos */
  if (*m_p <= 0 || *n_p <= 0 || *m_p % comm_sz != 0 || *n_p % comm_sz != 0)
    no_err_flag = 0; /* Flag que determina la valides de las dimensiones. */

  /* Se verifica si existe algún error acorde al flag. */
  hasErrors(no_err_flag, "setDim", "m y n deben ser positivos y divisibles entre comm_sz", comm);

  /* Se almacena la porción en la que trabajara cada proceso. */
  *local_m_p = *m_p / comm_sz;
  *local_n_p = *n_p / comm_sz;
}

void initLocalArrays(std::vector<double>& local_A, std::vector<double>& local_x, std::vector<double>& local_y, int local_m, int n, int local_n) {
  /* Se redimensiona las porciones con las que trabajara cada proceso. */
  local_A.resize(local_m * n);
  local_x.resize(local_n);
  local_y.resize(local_m);
}

void genRandomMatrix(std::vector<double>& local_A, int m, int local_m, int n, int my_rank, MPI_Comm comm) {
  std::vector<double> A;
  if (my_rank == 0) {
    A.resize(m * n);
    std::cout << "Generando matriz A con números aleatorios" << std::endl;
    for (int i = 0; i < m; i++)
      for (int j = 0; j < n; j++)
        A[i * n + j] = static_cast<double>(rand()) / RAND_MAX; // Genera números aleatorios entre 0 y 1
  }
  MPI_Scatter(A.data(), local_m * n, MPI_DOUBLE, local_A.data(), local_m * n, MPI_DOUBLE, 0, comm);
}

void genRandomVec(std::vector<double>& local_vec, int n, int local_n, int my_rank, MPI_Comm comm) {
  std::vector<double> vec;
  if (my_rank == 0) {
    vec.resize(n);
    std::cout << "Generando vector x con números aleatorios" << std::endl;
    for (int i = 0; i < n; i++)
      vec[i] = static_cast<double>(rand()) / RAND_MAX; // Genera números aleatorios entre 0 y 1
  }
  MPI_Scatter(vec.data(), local_n, MPI_DOUBLE, local_vec.data(), local_n, MPI_DOUBLE, 0, comm);
}

void printMatrix(const std::string& title, const std::vector<double>& local_A, int m, int local_m, int n, int my_rank, MPI_Comm comm) {
  std::vector<double> A;
  if (my_rank == 0) {
    A.resize(m * n);
  }
  MPI_Gather(local_A.data(), local_m * n, MPI_DOUBLE, A.data(), local_m * n, MPI_DOUBLE, 0, comm);
  if (my_rank == 0) {
    std::cout << "\nLa matriz " << title << ": " << std::endl;
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < n; j++) {
        std::cout << A[i * n + j] << " ";
      }
      std::cout << std::endl;
    }
  }
}

void printVector(const std::string& title, const std::vector<double>& local_vec, int n, int local_n, int my_rank, MPI_Comm comm) {
  std::vector<double> vec;
  if (my_rank == 0) {
    vec.resize(n);
  }
  MPI_Gather(local_vec.data(), local_n, MPI_DOUBLE, vec.data(), local_n, MPI_DOUBLE, 0, comm);
  if (my_rank == 0) {
    std::cout << "\nEl vector " << title << std::endl;
    for (int i = 0; i < n; i++)
      std::cout << vec[i] << " ";
    std::cout << std::endl;
  }
}

void matVecMult(
    const std::vector<double>& local_A, // Parte de la matriz A local en cada proceso
    const std::vector<double>& local_x, // Parte del vector x local en cada proceso
    std::vector<double>& local_y,       // Parte del resultado y local en cada proceso
    int local_m,                        // Número de filas de la matriz local en cada proceso
    int n,                              // Número de columnas de la matriz (tamaño completo del vector x)
    int local_n,                        // Número de elementos del vector x local en cada proceso
    MPI_Comm comm                       // Comunicador MPI
) {
  /* Vector resultante */
  std::vector<double> x(n);
  MPI_Allgather(local_x.data(), local_n, MPI_DOUBLE, x.data(), local_n, MPI_DOUBLE, comm);
  for (int local_i = 0; local_i < local_m; local_i++) {
    local_y[local_i] = 0.0;
    for (int j = 0; j < n; j++)
      local_y[local_i] += local_A[local_i * n + j] * x[j];
  }
}

int main()
{
  std::vector<double> local_A;
  std::vector<double> local_x;
  std::vector<double> local_y;
  int m, local_m, n, local_n;
  int my_rank, comm_sz;
  MPI_Comm comm;
  double start, finish, loc_elapsed, elapsed;
  const std::string default_message = "@%s: Hello from process [ %i ] out of [ %i ] processes.\n";

  MPI_Init(nullptr, nullptr);
  comm = MPI_COMM_WORLD;
  MPI_Comm_size(comm, &comm_sz);
  MPI_Comm_rank(comm, &my_rank);

  int name_len;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  MPI_Get_processor_name(processor_name, &name_len);
  printf(default_message.c_str(), processor_name, my_rank, comm_sz);
  MPI_Barrier(comm); // Sincroniza los procesos antes de continuar.


  /* Inicializa la semilla del generador de números aleatorios */
  std::srand(static_cast<unsigned>(std::time(nullptr)) + my_rank);

  setDim(&m, &local_m, &n, &local_n, my_rank, comm_sz, comm);
  initLocalArrays(local_A, local_x, local_y, local_m, n, local_n);

  genRandomMatrix(local_A, m, local_m, n, my_rank, comm);
  genRandomVec(local_x, n, local_n, my_rank, comm);

  #ifdef DEBUG
  printMatrix("A", local_A, m, local_m, n, my_rank, comm);
  printVector("x", local_x, n, local_n, my_rank, comm);
  #endif

  MPI_Barrier(comm);
  start = MPI_Wtime();
  matVecMult(local_A, local_x, local_y, local_m, n, local_n, comm);
  finish = MPI_Wtime();
  loc_elapsed = finish-start;
  MPI_Reduce(&loc_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, comm);

  #ifdef DEBUG
  printVector("y", local_y, m, local_m, my_rank, comm);
  #endif

  if (my_rank == 0) {
    std::cout << "Elapsed time = " << elapsed << "\n";
  }

  MPI_Finalize();
  return 0;
}

// mpiexec --allow-run-as-root --hostfile hosts ./mpi_mat_vect_mult