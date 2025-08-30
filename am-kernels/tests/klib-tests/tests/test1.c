#include "trap.h"
#include <klib.h>
#include <stdio.h>

#define NR_DATA LENGTH(test_data)
const char *a = "1145141919810";
const char *b = "1919810";
const char *c = "1145141919810";
const char *d = "514";
int test_strcmp() {

  int ret = strcmp(a, b);
  if (ret == 0)
    return ret;
  ret = strcmp(a, c);
  if (ret != 0)
    return ret;
  ret = strcmp(a, d);
  if (ret == 0)
    return ret;
  ret = strcmp(a + 3, d);
  if (ret == 0)
    return ret;
  ret = strcmp(c + 6, b);
  if (ret != 0)
    return ret;
  return 0;
}

int test_memcmp() {
  int ret = memcmp(a, b, 4);
  if (ret == 0)
    return ret;
  ret = memcmp(a, c, 10);
  if (ret != 0)
    return ret;
  ret = memcmp(a, d, 3);
  if (ret == 0)
    return ret;
  ret = memcmp(a + 3, d, 3);
  if (ret != 0)
    return ret;
  ret = memcmp(c + 6, b, 4);
  if (ret != 0)
    return ret;
  return 0;
}

int test_memmove() {
  int x[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  int y[8] = {};
  int z[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  const int r1[8] = {3, 4, 5, 6, 0, 0, 0, 0};
  const int r2[8] = {3, 4, 5, 6, 7, 6, 7, 8};
  const int r3[8] = {1, 2, 1, 2, 3, 4, 5, 8};
  memmove(y, x + 2, 4 * sizeof(int));
  for (int i = 0; i < 8; i++)
    if (y[i] != r1[i])
      return -1;
  memmove(x, x + 2, 5 * sizeof(int));
  for (int i = 0; i < 8; i++)
    if (x[i] != r2[i])
      return -1;
  memmove(z + 2, z, 5 * sizeof(int));
  for (int i = 0; i < 8; i++)
    if (z[i] != r3[i])
      return -1;
  return 0;
}

int main() {
  int ret = test_strcmp();
  if (ret != 0)
    return ret;
  ret = test_memcmp();
  if (ret != 0)
    return ret;
  ret = test_memmove();
  if (ret != 0)
    return ret;
  return 0;
}
