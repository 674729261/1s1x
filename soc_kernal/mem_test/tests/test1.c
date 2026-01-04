#include <klib.h>
#include <stdint.h>

#define NR_DATA LENGTH(test_data)

#define MY_CHECK(C)                                                            \
  do {                                                                         \
    if (!(C))                                                                  \
      return -1;                                                               \
  } while (0)

#define TEST_ADDR_START 0x0f000000
#define TEST_ADDR_END 0x0f001000

int test_word() {
  uint32_t *start = (uint32_t *)(0x0f000000);
  uint32_t *end = (uint32_t *)(0x0f001000);
  for (volatile uint32_t *p = start; p < end; p++) {
    *p = (uint32_t)((uint32_t)p * 23);
  }
  for (volatile uint32_t *p = start; p < end; p++) {
    *p = (uint32_t)p;
    MY_CHECK(*p == (uint32_t)((uint32_t)p * 23));
  }
  return 0;
}

int main() {
  int return_value = test_word();
  if (return_value)
    return -1;
  return 0;
}
