#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

long long f(long long n, long long m) { return f(n, m); }

int main(int argc, char *argv[]) {
  assert(argc == 3);
  long long n = atoll(argv[1]);
  long long m = atoll(argv[2]);
  printf("%lld\n", f(n, m));
}