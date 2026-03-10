#include <stdio.h>
#include <thread>
#include "lib.h"

double input[MAXN][MAXN];
double output[MAXN][MAXN];
double inputB[MAXN][MAXN];
double outputB[MAXN][MAXN];

int main(int argc, char* argv[]) {
  int N_values[] = {256, 512, 1024};
  int B_values[] = {8, 16, 32, 64};

  FILE* fp = fopen("results.csv", "w");
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

  fclose(fp);
  return 0;
}
