#include <iostream>
#include <mpi.h>

int main (int argc, char *argv[])
{
    MPI_Init (&argc, &argv);

    int stat, rank, size;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    char errMessage[100];
    int errMessLen;
    if (rank == 1) {
        MPI_Comm_set_errhandler (MPI_COMM_WORLD, MPI_ERRORS_RETURN);
        if ((stat = MPI_Init (&argc, &argv) & 0xFF) != MPI_SUCCESS) {
            switch (stat) {
                case MPI_ERR_COMM:
                    std::cerr << "Invalid communicator!" << std::endl;
                    break;
                case MPI_ERR_OTHER:
                    MPI_Error_string (stat, errMessage, &errMessLen);
                    std::cerr << errMessage
                              << ": MPI_Init called more than once!" << std::endl;
                    break;
            }
            MPI_Abort (MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    std::cout << "Hello! I am " << rank << " of " << size << std::endl;
    MPI_Finalize ();
    return EXIT_SUCCESS;
}
