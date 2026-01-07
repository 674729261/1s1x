#include <klib.h>
#include <limits.h>
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

int test_memcpy() {
  char x[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  char y[8] = {};
  const char r1[8] = {0, 3, 4, 5, 6, 7, 8, 0};
  const char r2[8] = {0, 3, 4, 5, 7, 4, 5, 0};
  memcpy(y + 1, x + 2, 6 * sizeof(char));
  MY_RANGE(y, r1, 0, 8);
  memcpy(y + 5, x + 3, 2 * sizeof(char));
  MY_RANGE(y, r2, 0, 8);
  return 0;
}

int test_memset() {
  int x[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  const int r1[8] = {-1, -1, -1, -1, -1, 6, 7, 8};
  memset(x, -1, 5 * sizeof(int));
  MY_RANGE(x, r1, 0, 8);

  char y[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
  memset(y + 5, -5, 9 * sizeof(char));
  const char r2[16] = {1,  2,  3,  4,  5,  -5, -5, -5,
                       -5, -5, -5, -5, -5, -5, 15, 16};
  MY_RANGE(y, r2, 0, 8);

  memset(y + 1, -7, 2 * sizeof(char));
  const char r3[16] = {1,  -7, -7, 4,  5,  -5, -5, -5,
                       -5, -5, -5, -5, -5, -5, 15, 16};
  MY_RANGE(y, r3, 0, 8);

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

int test_sprintf() {
  char x[128] = {};
  int a = 42;
  unsigned long long b = 0x12345abcdef;
  const char *s = "foobar";

  sprintf(x,
          "MX is %d\nMN is %d\nZERO is %d\nMNdiv7 is %d\na is %+04d\nb is "
          "%#018llx\ns is %s\n",
          INT_MAX, INT_MIN, 0, INT_MIN / 7, a, b, s);
  MY_RANGE(x,
           "MX is 2147483647\nMN is -2147483648\nZERO is 0\nMNdiv7 is "
           "-306783378\na is "
           "+042\nb is "
           "0x0000012345abcdef\ns is foobar\n",
           0, 112);
  return 0;
}

int test_strlen() {
  const char *x = "abcdef123456\n\0\0abcd\0\0";
  const char *y = "";
  const char *z = "123";
  int a = strlen(x);
  MY_CHECK(a == 13);
  a = strlen(y);
  MY_CHECK(a == 0);
  a = strlen(z);
  MY_CHECK(a == 3);
  return 0;
}

int main() {
  int ret = test_strcmp();
  MY_CHECK(ret == 0);
  ret = test_memcmp();
  MY_CHECK(ret == 0);
  ret = test_memmove();
  MY_CHECK(ret == 0);
  ret = test_memcpy();
  MY_CHECK(ret == 0);
  ret = test_strcat();
  MY_CHECK(ret == 0);
  ret = test_memset();
  MY_CHECK(ret == 0);
  ret = test_sprintf();
  MY_CHECK(ret == 0);
  ret = test_strlen();
  MY_CHECK(ret == 0);
  return 0;
}
