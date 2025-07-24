#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#define SHOW(T) printf("%18s : size =%2zu\n", #T, sizeof(T))
int main(void) {
  SHOW(int);
  SHOW(unsigned int);
  SHOW(long long);
  SHOW(unsigned long long);
  SHOW(long);
  SHOW(unsigned long);
  SHOW(short);
  SHOW(unsigned short);
  SHOW(char);
  SHOW(unsigned char);
  SHOW(size_t);
}