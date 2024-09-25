#include <iostream>
#include <cstdlib>
#include <cstring>
#include <mpi.h>
#include <ctime>

const int RMAX = 100;

/* Local functions */
void Print_list(int local_A[], int local_n, int rank);
void Merge_low(int local_A[], int temp_B[], int temp_C[], int local_n);
void Merge_high(int local_A[], int temp_B[], int temp_C[], int local_n);
void Generate_list(int local_A[], int local_n, int my_rank);
int Compare(const void* a_p, const void* b_p);

/* Functions involving communication */
void Get_args(int argc, char* argv[], int* global_n_p, int* local_n_p, int my_rank, int p, MPI_Comm comm);
void Sort(int local_A[], int local_n, int my_rank, int p, MPI_Comm comm);
void Odd_even_iter(int local_A[], int temp_B[], int temp_C[], int local_n, int phase, int even_partner, int odd_partner, int my_rank, int p, MPI_Comm comm);
void Print_local_lists(int local_A[], int local_n, int my_rank, int p, MPI_Comm comm);
void Print_global_list(int local_A[], int local_n, int my_rank, int p, MPI_Comm comm);

int main(int argc, char* argv[]) {
    int my_rank, p;
    int *local_A;
    int global_n;
    int local_n;
    MPI_Comm comm;
    double start, finish, loc_elapsed, elapsed;
    const std::string default_message = "@%s: Hello from process [ %i ] out of [ %i ] processes.\n";

    MPI_Init(&argc, &argv);
    comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &p);
    MPI_Comm_rank(comm, &my_rank);

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    printf(default_message.c_str(), processor_name, my_rank, p);
    MPI_Barrier(comm);

    Get_args(argc, argv, &global_n, &local_n, my_rank, p, comm);
    local_A = new int[local_n];

    Generate_list(local_A, local_n, my_rank);
    #ifdef DEBUG
    Print_local_lists(local_A, local_n, my_rank, p, comm);
    #endif

    MPI_Barrier(comm);
    start = MPI_Wtime();
    Sort(local_A, local_n, my_rank, p, comm);
    finish = MPI_Wtime();
    loc_elapsed = finish-start;
    MPI_Reduce(&loc_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, comm);

    #ifdef DEBUG
    Print_global_list(local_A, local_n, my_rank, p, comm);
    #endif


    if (my_rank == 0) {
        std::cout << "Elapsed time = " << elapsed << "\n";
    }

    delete[] local_A;
    MPI_Finalize();
    return 0;
}

/*-------------------------------------------------------------------
 * Function:   Generate_list
 * Purpose:    Fill list with random ints
 * Input Args: local_n, my_rank
 * Output Arg: local_A
 */
void Generate_list(int local_A[], int local_n, int my_rank) {
    std::srand(my_rank + std::time(0));
    for (int i = 0; i < local_n; i++) {
        local_A[i] = std::rand() % RMAX;
    }
}

/*-------------------------------------------------------------------
 * Function:    Get_args
 * Purpose:     Get and check command line arguments
 * Input args:  argc, argv, my_rank, p, comm
 * Output args: global_n_p, local_n_p
 */
void Get_args(int argc, char* argv[], int* global_n_p, int* local_n_p, int my_rank, int p, MPI_Comm comm) {
    if (my_rank == 0) {
        if (argc != 2) {
            std::cerr << "Usage: mpirun -np <p> " << argv[0] << " <global_n>\n";
            *global_n_p = -1;
        } else {
            *global_n_p = std::atoi(argv[1]);
            if (*global_n_p % p != 0) {
                std::cerr << "global_n must be evenly divisible by p\n";
                *global_n_p = -1;
            }
        }
    }

    MPI_Bcast(global_n_p, 1, MPI_INT, 0, comm);

    if (*global_n_p <= 0) {
        MPI_Finalize();
        exit(-1);
    }

    *local_n_p = *global_n_p / p;
}

/*-------------------------------------------------------------------
 * Function:   Print_global_list
 * Purpose:    Print the contents of the global list A
 */
void Print_global_list(int local_A[], int local_n, int my_rank, int p, MPI_Comm comm) {
    int* A = nullptr;
    if (my_rank == 0) {
        A = new int[p * local_n];
        MPI_Gather(local_A, local_n, MPI_INT, A, local_n, MPI_INT, 0, comm);
        std::cout << "Global list:\n";
        for (int i = 0; i < p * local_n; i++) {
            std::cout << A[i] << " ";
        }
        std::cout << "\n\n";
        delete[] A;
    } else {
        MPI_Gather(local_A, local_n, MPI_INT, A, local_n, MPI_INT, 0, comm);
    }
}

/*-------------------------------------------------------------------
 * Function:    Compare
 * Purpose:     Compare two ints, used by qsort
 */
int Compare(const void* a_p, const void* b_p) {
    int a = *((int*)a_p);
    int b = *((int*)b_p);
    return (a > b) - (a < b);
}

/*-------------------------------------------------------------------
 * Function:    Sort
 * Purpose:     Sort local list, use odd-even sort to sort global list.
 */
void Sort(int local_A[], int local_n, int my_rank, int p, MPI_Comm comm) {
    int phase;
    int *temp_B = new int[local_n];
    int *temp_C = new int[local_n];
    int even_partner, odd_partner;

    if (my_rank % 2 != 0) {
        even_partner = my_rank - 1;
        odd_partner = my_rank + 1;
        if (odd_partner == p) odd_partner = MPI_PROC_NULL;
    } else {
        even_partner = my_rank + 1;
        if (even_partner == p) even_partner = MPI_PROC_NULL;
        odd_partner = my_rank - 1;
    }

    std::qsort(local_A, local_n, sizeof(int), Compare);

    for (phase = 0; phase < p; phase++) {
        Odd_even_iter(local_A, temp_B, temp_C, local_n, phase, even_partner, odd_partner, my_rank, p, comm);
    }

    delete[] temp_B;
    delete[] temp_C;
}

/*-------------------------------------------------------------------
 * Function:    Odd_even_iter
 * Purpose:     One iteration of Odd-even transposition sort
 */
void Odd_even_iter(int local_A[], int temp_B[], int temp_C[], int local_n, int phase, int even_partner, int odd_partner, int my_rank, int p, MPI_Comm comm) {
    MPI_Status status;
    if (phase % 2 == 0) {
        if (even_partner >= 0) {
            MPI_Sendrecv(local_A, local_n, MPI_INT, even_partner, 0, temp_B, local_n, MPI_INT, even_partner, 0, comm, &status);
            if (my_rank % 2 != 0)
                Merge_high(local_A, temp_B, temp_C, local_n);
            else
                Merge_low(local_A, temp_B, temp_C, local_n);
        }
    } else {
        if (odd_partner >= 0) {
            MPI_Sendrecv(local_A, local_n, MPI_INT, odd_partner, 0, temp_B, local_n, MPI_INT, odd_partner, 0, comm, &status);
            if (my_rank % 2 != 0)
                Merge_low(local_A, temp_B, temp_C, local_n);
            else
                Merge_high(local_A, temp_B, temp_C, local_n);
        }
    }
}

/*-------------------------------------------------------------------
 * Function:    Merge_low
 * Purpose:     Merge the smallest local_n elements into local_A
 */
void Merge_low(int local_A[], int temp_B[], int temp_C[], int local_n) {
    int m_i = 0, r_i = 0, t_i = 0;
    while (t_i < local_n) {
        if (local_A[m_i] <= temp_B[r_i]) {
            temp_C[t_i++] = local_A[m_i++];
        } else {
            temp_C[t_i++] = temp_B[r_i++];
        }
    }
    std::memcpy(local_A, temp_C, local_n * sizeof(int));
}

/*-------------------------------------------------------------------
 * Function:    Merge_high
 * Purpose:     Merge the largest local_n elements into local_A
 */
void Merge_high(int local_A[], int temp_B[], int temp_C[], int local_n) {
    int ai = local_n - 1, bi = local_n - 1, ci = local_n - 1;
    while (ci >= 0) {
        if (local_A[ai] >= temp_B[bi]) {
            temp_C[ci--] = local_A[ai--];
        } else {
            temp_C[ci--] = temp_B[bi--];
        }
    }
    std::memcpy(local_A, temp_C, local_n * sizeof(int));
}

/*-------------------------------------------------------------------
 * Function:    Print_local_lists
 * Purpose:     Have each process print its current list
 */
void Print_local_lists(int local_A[], int local_n, int my_rank, int p, MPI_Comm comm) {
    int* A = nullptr;
    if (my_rank == 0) {
        A = new int[p * local_n];
        MPI_Gather(local_A, local_n, MPI_INT, A, local_n, MPI_INT, 0, comm);
        for (int i = 0; i < p; i++) {
            std::cout << "Process " << i << ": ";
            for (int j = 0; j < local_n; j++) {
                std::cout << A[i * local_n + j] << " ";
            }
            std::cout << "\n";
        }
        delete[] A;
    } else {
        MPI_Gather(local_A, local_n, MPI_INT, A, local_n, MPI_INT, 0, comm);
    }
}
