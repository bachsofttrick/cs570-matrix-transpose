#include <time.h>
#include <algorithm>
#define MAXN 1024
#define T 10000

double get_seconds();
void non_blocking(double output[MAXN][MAXN], double input[MAXN][MAXN], int n, double* result);
void blocking(double output[MAXN][MAXN], double input[MAXN][MAXN], int n, int b, double* result);
void non_blocking_pointer(double** output, double** input, int n, double* result);
void blocking_pointer(double** output, double** input, int n, int b, double* result);