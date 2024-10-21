#include <mpi.h>
#include <time.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  int n_samples = 8193;
  double c_re_min = -2.05;
  double c_re_max = +0.55;
  double c_im_min = -1.3;
  double c_im_max = +1.3;
  int global_count = 0;
  clock_t t;

  // Inicializar o ambiente MPI
  MPI_Init(&argc, &argv);
  int rank, n_processes;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &n_processes);

  // Dividir o trabalho entre os processos
  int local_count = 0;
  int rows_per_process = n_samples / n_processes;
  int start_row = rank * rows_per_process;
  int end_row = (rank == n_processes - 1) ? n_samples : start_row + rows_per_process;

  t = clock();
  for (int re_idx = start_row; re_idx < end_row; re_idx++)
  {
    double c_re = c_re_min + (double)re_idx * ((c_re_max - c_re_min) / (double)n_samples);
    for (int im_idx = 0; im_idx < n_samples; im_idx++)
    {
      double c_im = c_im_min + (double)im_idx * ((c_im_max - c_im_min) / (double)n_samples);
      int n_iter = 0;
      double z_re = 0.0;
      double z_im = 0.0;
      while (n_iter < 256 && z_re * z_re + z_im * z_im <= 4.0)
      {
        double tmp = z_re * z_re - z_im * z_im;
        z_im = 2.0 * z_re * z_im + c_im;
        z_re = tmp + c_re;
        n_iter++;
      }
      if (n_iter < 256)
        local_count++;
    }
  }
  t = clock() - t;

  // Reduzir os resultados parciais para obter o resultado global
  MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  // O processo com rank 0 imprime o resultado
  if (rank == 0)
  {
    printf("%d -- %7.3f\n", global_count, (double)t / (double)CLOCKS_PER_SEC);
  }

  // Finalizar o ambiente MPI
  MPI_Finalize();
  return 0;
}