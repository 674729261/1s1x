#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

long long f(long long n, long long m) {
  if (n == 0)
    return 1;
  if (m < n)
    return 0;
  return f(n, m - 1) + f(n - 1, m - 1);
}

int main(int argc, char *argv[]) {
  assert(argc == 3);
  long long n = atoll(argv[1]);
  long long m = atoll(argv[2]);
  printf("%lld\n", f(n, m));
}