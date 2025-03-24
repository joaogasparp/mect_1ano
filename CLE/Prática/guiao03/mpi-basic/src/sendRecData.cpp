#include <iostream>
#include <cstring>
#include <mpi.h>

int main (int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        char data[] = "I am here!";
        std::cout << "Transmitted message: " << data << std::endl;
        MPI_Send(data, std::strlen (data), MPI_CHAR, 1, 0, MPI_COMM_WORLD);
    } else if (rank == 1) {
        int i;
        char* recData = new char(100);
        for (i = 0; i < 100; i++)
            recData[i] = '\0';

        MPI_Recv(recData, 100, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Received message: " << recData << std::endl;
    }

    MPI_Finalize ();
    return EXIT_SUCCESS;
}
