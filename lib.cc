#include "lib.h"

double get_seconds() {
  time_t res = time(NULL);
  return (double) res;
}

void non_blocking(double output[MAXN][MAXN], double input[MAXN][MAXN], int n, double* result) {
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

void blocking(double output[MAXN][MAXN], double input[MAXN][MAXN], int n, int b, double* result) {
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


void non_blocking_pointer(double** output, double** input, int n, double* result) {
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

void blocking_pointer(double** output, double** input, int n, int b, double* result) {
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