#include <stdio.h>
#include <time.h>
#include <unistd.h>

double get_time(int i) {
  clock_t clk = clock();
  double ret = (double)clk / CLOCKS_PER_SEC;
  return ret;
}

int main(int argc, char *argv[]) {
  int i = 0;

  while (i < 100) {
    usleep(300000);
    printf("time = %fs, i = %d\n", get_time(i), i);
  }
  printf("time() = %ld, i = %d\n, jumped out\n", clock(), i);
}