#include <stdio.h>
#include <time.h>
#include <thread>
#include <algorithm>
#define N 512
#define B 8
#define T 10000

using namespace std;

double input[N][N];
double output[N][N];
double inputB[N][N];
double outputB[N][N];

double get_seconds() {
  time_t res = time(NULL);
  return (double) res;
} 

void print_matrix(double matrix[N][N],int size) {
    int i,j;
    for(i=0;i<size;i++) {
        for(j=0;j<size;j++) {
            printf("%1.0f ",matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void non_blocking(double output[N][N], double input[N][N]) {
  double time0, time1, tdiff;
  time0 = get_seconds();

  for(int trials=0;trials<T;trials++) {
    for(int i=0;i<N;i++) {
      for(int j=0;j<N;j++) {
        output[j][i]=input[i][j];
      }
    }
  }

  time1 = get_seconds();
  printf("Non-blocking:\n");
  print_matrix(output,10);
  tdiff = time1-time0;
  printf("Time1-time0: %lf\n", tdiff);
}

void blocking(double output[N][N], double input[N][N]) {
  double time0, time1, tdiff;
  time0 = get_seconds();

  for(int trials=0;trials<T;trials++) {
    for(int m=0;m<N;m+=B) {
      for(int n=0;n<N;n+=B) {
        for(int i=m;i<min(m+B, N);i++) {
      	  for(int j=n;j<min(n+B, N);j++) {
        	  output[j][i]=input[i][j];
      	  }
    	  }
      }
    }
  }

  time1 = get_seconds();
  printf("Blocking:\n");
  print_matrix(output,10);
  tdiff = time1-time0;
  printf("Time1-time0: %lf\n", tdiff);
}

int main(int argc, char* argv[]) {
  int i,j,m,n;
  int trials;
  double time0, time1, time2, time3, tdiff;

  for(i=0;i<N;i++) {
    for(j=0;j<N;j++) {
        input[i][j]=i;
        inputB[i][j]=i;
        output[i][j]=0.0;
        outputB[i][j]=0.0;
    }
  }
  print_matrix(input,10);
  
  thread thread1(non_blocking, output, input);
  thread thread2(blocking, outputB, inputB);

  // Wait for both threads to complete
  thread1.join();
  thread2.join();

  return 0;
}
