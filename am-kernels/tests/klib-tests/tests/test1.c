#include "trap.h"
#include <klib.h>
#include <stdio.h>

#define NR_DATA LENGTH(test_data)

#define MY_CHECK(C)                                                            \
  do {                                                                         \
    if (!(C))                                                                  \
      return -1;                                                               \
  } while (0)

#define MY_RANGE(A, B, F, T)                                                   \
  do                                                                           \
    for (int qwertqwert__ = (F); qwertqwert__ < (T); qwertqwert__++)           \
      if ((A)[qwertqwert__] != (B)[qwertqwert__])                              \
        return -1;                                                             \
  while (0)

const char *a = "1145141919810";
const char *b = "1919810";
const char *c = "1145141919810";
const char *d = "514";
int test_strcmp() {

  int ret = strcmp(a, b);
  MY_CHECK(ret != 0);
  ret = strcmp(a, c);
  MY_CHECK(ret == 0);
  ret = strcmp(a, d);
  MY_CHECK(ret != 0);
  ret = strcmp(a + 3, d);
  MY_CHECK(ret != 0);
  ret = strcmp(c + 6, b);
  MY_CHECK(ret == 0);
  return 0;
}

int test_memcmp() {
  int ret = memcmp(a, b, 4);
  MY_CHECK(ret != 0);
  ret = memcmp(a, c, 10);
  MY_CHECK(ret == 0);
  ret = memcmp(a, d, 3);
  MY_CHECK(ret != 0);
  ret = memcmp(a + 3, d, 3);
  MY_CHECK(ret == 0);
  ret = memcmp(c + 6, b, 4);
  MY_CHECK(ret == 0);
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
  MY_RANGE(y, r1, 0, 8);
  memmove(x, x + 2, 5 * sizeof(int));
  MY_RANGE(x, r2, 0, 8);
  memmove(z + 2, z, 5 * sizeof(int));
  MY_RANGE(z, r3, 0, 8);
  return 0;
}

int test_strcat() {
  char x[8] = "abcd";
  char y[4] = "efg";

  strcat(x, y);
  MY_RANGE(x, "abcdefg", 0, 8);
  strcat(y, "");
  MY_RANGE(y, "efg", 0, 4);
  return 0;
}

int main() {
  int ret = test_strcmp();
  MY_CHECK(ret == 0);
  ret = test_memcmp();
  MY_CHECK(ret == 0);
  ret = test_memmove();
  MY_CHECK(ret == 0);
  ret = test_strcat();
  MY_CHECK(ret == 0);
  return 0;
}
