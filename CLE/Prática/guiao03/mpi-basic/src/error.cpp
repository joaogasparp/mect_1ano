#include <iostream>
#include <mpi.h>

int main (int argc, char *argv[])
{
    MPI_Init (&argc, &argv);

    int rank, size;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    if (rank == 1)
        MPI_Init (&argc, &argv); // <- error! can only be called once

    std::cout << "Hello! I am " << rank << " of " << size << std::endl;

    MPI_Finalize();
    return EXIT_SUCCESS;
}
