#include <stdio.h>
#include <time.h>
#include <thread>
#include <algorithm>
#define MAXN 1024
#define T 10000

double get_seconds() {
  time_t res = time(NULL);
  return (double) res;
}

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

void non_blocking(double** output, double** input, int n, double* result) {
  double time0 = get_seconds();

  for(int trials=0;trials<T;trials++) {
    for(int i=0;i<n;i++) {
      for(int j=0;j<n;j++) {
        output[j][i]=input[i][j];
      }
    }
  }

  *result = get_seconds() - time0;
}

void blocking(double** output, double** input, int n, int b, double* result) {
  double time0 = get_seconds();

  for(int trials=0;trials<T;trials++) {
    for(int m=0;m<n;m+=b) {
      for(int k=0;k<n;k+=b) {
        for(int i=m;i<std::min(m+b, n);i++) {
          for(int j=k;j<std::min(k+b, n);j++) {
            output[j][i]=input[i][j];
          }
        }
      }
    }
  }

  *result = get_seconds() - time0;
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
      std::thread thread1(non_blocking, output, input, n, &t_nb);
      std::thread thread2(blocking, outputB, inputB, n, b, &t_b);

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
