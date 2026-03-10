#include <stdio.h>
#include <thread>
#include "lib.h"

double** alloc_matrix(int n) {
  double** m = new double*[n];
  for(int i=0;i<n;i++) m[i] = new double[n];
  return m;
}

void free_matrix(double** m, int n) {
  for(int i=0;i<n;i++) delete[] m[i];
  delete[] m;
}

void create_matrix(double** output, double** input, double** outputB, double** inputB, int n) {

}

void erase_matrix(double** output, double** input, double** outputB, double** inputB, int n) {

}

int main(int argc, char* argv[]) {
  int N_values[] = {256, 512, 1024};
  int B_values[] = {8, 16, 32, 64};

  double** input   = alloc_matrix(MAXN);
  double** inputB  = alloc_matrix(MAXN);
  double** output  = alloc_matrix(MAXN);
  double** outputB = alloc_matrix(MAXN);

  FILE* fp = fopen("results_malloc.csv", "w");
  fprintf(fp, "N;B;non-blocking (s);blocking (s)\n");

  for(int ni=0;ni<sizeof(N_values)/sizeof(N_values[0]);ni++) {
    int n = N_values[ni];

    for(int bi=0;bi<sizeof(B_values)/sizeof(B_values[0]);bi++) {
      int b = B_values[bi];

      for(int i=0;i<n;i++) {
        for(int j=0;j<n;j++) {
          input[i][j]=i;
          inputB[i][j]=i;
          output[i][j]=0.0;
          outputB[i][j]=0.0;
        }
      }

      double t_nb, t_b;
      std::thread thread1(non_blocking_pointer, output, input, n, &t_nb);
      std::thread thread2(blocking_pointer, outputB, inputB, n, b, &t_b);

      thread1.join();
      thread2.join();

      fprintf(fp, "%d;%d;%lf;%lf\n", n, b, t_nb, t_b);
    }
  }

  free_matrix(input, MAXN);
  free_matrix(inputB, MAXN);
  free_matrix(output, MAXN);
  free_matrix(outputB, MAXN);

  fclose(fp);
  return 0;
}
