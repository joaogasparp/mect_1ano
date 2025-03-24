#include <iostream>
#include <cstring>
#include <mpi.h>
#include <cstdlib>
#include <ctime>
#include <limits>

const int MAX_SIZE = 1000;

void exercise_3_1(int rank, int size)
{
    char message[MAX_SIZE];
    memset(message, 0, MAX_SIZE);

    if (rank == 0)
    {
        snprintf(message, MAX_SIZE, "I am here (%d)!\n", rank);
        MPI_Send(message, strlen(message) + 1, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
        MPI_Recv(message, MAX_SIZE, MPI_CHAR, size - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "[P0] Mensagem final:\n"
                  << message << std::endl;
    }
    else
    {
        MPI_Recv(message, MAX_SIZE, MPI_CHAR, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        char myPart[100];
        snprintf(myPart, sizeof(myPart), "I am here (%d)!\n", rank);
        strncat(message, myPart, MAX_SIZE - strlen(message) - 1);
        int next = (rank + 1) % size;
        MPI_Send(message, strlen(message) + 1, MPI_CHAR, next, 0, MPI_COMM_WORLD);
    }
}

void exercise_3_2(int rank, int size)
{
    const int MSG_LEN = 100;
    if (rank == 0)
    {
        const char *msg = "alive and well";
        for (int dest = 1; dest < size; ++dest)
        {
            MPI_Send(msg, strlen(msg) + 1, MPI_CHAR, dest, 0, MPI_COMM_WORLD);
        }

        for (int i = 1; i < size; ++i)
        {
            char buffer[MSG_LEN];
            MPI_Recv(buffer, MSG_LEN, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::cout << "[P0] Received: " << buffer;
        }
    }
    else
    {
        char buffer[MSG_LEN];
        MPI_Recv(buffer, MSG_LEN, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        char response[MSG_LEN];
        snprintf(response, MSG_LEN, "Process %d is alive and well\n", rank);
        MPI_Send(response, strlen(response) + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }
}

void exercise_3_3(int rank, int size)
{
    const int NUM_VALUES = 10; // número de valores por processo
    int localValues[NUM_VALUES];

    // Inicializar random com seed diferente por processo
    srand(time(nullptr) + rank);

    // Gerar números aleatórios entre 0 e 999
    for (int i = 0; i < NUM_VALUES; ++i)
    {
        localValues[i] = rand() % 1000;
    }

    // Calcular min e max local
    int localMin = localValues[0];
    int localMax = localValues[0];
    for (int i = 1; i < NUM_VALUES; ++i)
    {
        if (localValues[i] < localMin)
            localMin = localValues[i];
        if (localValues[i] > localMax)
            localMax = localValues[i];
    }

    // Redução para encontrar global min e max
    int globalMin, globalMax;
    MPI_Reduce(&localMin, &globalMin, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&localMax, &globalMax, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    // Só o processo 0 imprime os resultados globais
    if (rank == 0)
    {
        std::cout << "[P0] Global Min: " << globalMin << " | Global Max: " << globalMax << std::endl;
    }
}

void exercise_3_4(int rank, int size)
{
    const int N = 4; // Tamanho da matriz (4x4)
    const int rowsPerProcess = N / size;

    int A[N][N], B[N][N], C[N][N]; // Matrizes completas (apenas usadas no rank 0)
    int localA[rowsPerProcess][N]; // Linhas atribuídas a este processo
    int localC[rowsPerProcess][N]; // Resultado parcial
    memset(localC, 0, sizeof(localC));

    // Processo 0 inicializa as matrizes
    if (rank == 0)
    {
        // Exemplo simples: A = matriz identidade × 2, B = matriz de 1s
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                A[i][j] = (i == j) ? 2 : 0;
                B[i][j] = 1;
            }
        }
    }

    // Distribuir linhas da matriz A
    MPI_Scatter(A, rowsPerProcess * N, MPI_INT, localA, rowsPerProcess * N, MPI_INT, 0, MPI_COMM_WORLD);

    // Difundir a matriz B para todos os processos
    MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);

    // Cada processo calcula as suas linhas de C
    for (int i = 0; i < rowsPerProcess; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            localC[i][j] = 0;
            for (int k = 0; k < N; ++k)
            {
                localC[i][j] += localA[i][k] * B[k][j];
            }
        }
    }

    // Recolher o resultado completo no processo 0
    MPI_Gather(localC, rowsPerProcess * N, MPI_INT, C, rowsPerProcess * N, MPI_INT, 0, MPI_COMM_WORLD);

    // Imprimir o resultado no processo 0
    if (rank == 0)
    {
        std::cout << "[P0] Matriz resultado C = A × B:\n";
        for (int i = 0; i < N; ++i)
        {
            std::cout << "| ";
            for (int j = 0; j < N; ++j)
            {
                std::cout << C[i][j] << " ";
            }
            std::cout << "|\n";
        }
    }
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int exercise = 0;
    if (argc > 1)
    {
        exercise = std::atoi(argv[1]);
    }

    if (exercise == 1)
    {
        if (rank == 0)
            std::cout << "[ Exercício 3.1 - Ring Communication ]\n";
        exercise_3_1(rank, size);
    }
    else if (exercise == 2)
    {
        if (rank == 0)
            std::cout << "[ Exercício 3.2 - Alive and Well ]\n";
        exercise_3_2(rank, size);
    }
    else if (exercise == 3)
    {
        if (rank == 0)
            std::cout << "[ Exercício 3.3 - Global Min and Max ]\n";
        exercise_3_3(rank, size);
    }
    else if (exercise == 4)
    {
        if (rank == 0)
            std::cout << "[ Exercício 3.4 - Parallel Matrix Product ]\n";
        exercise_3_4(rank, size);
    }
    else
    {
        if (rank == 0)
        {
            std::cout << "Exercício inválido: " << exercise << std::endl;
            std::cout << "Usage: mpirun -np <num_processes> ./build/sendRecData <exercise_number>\n";
        }
    }

    MPI_Finalize();
    return 0;
}
