#include "dbg.h"
#include <stdarg.h>
#include <stdio.h>
int myprintf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  int cnt = 0;
  unsigned int v;
  char buffer[128];
  int len_deci;
  for (const char *p = format; *p != '\0'; p++) {
    if (*p == '%') {
      p++;
      switch (*p) {
      case '\0':
        sentinel("Invalid format, you ended with %%.");
        break;
      case 'b':
        v = va_arg(args, unsigned int);
        if (v == 0) {
          fputc('0', stdout);
          break;
        }
        len_deci = 0;
        while (v != 0) {
          buffer[len_deci++] = (v % 2 == 0 ? '0' : '1');
          v /= 2;
        }
        while (len_deci >= 1) {
          fputc(buffer[--len_deci], stdout);
        }
        break;
      default:
        sentinel("Invalid format.");
      }
    } else {
      fputc(*p, stdout);
    }
  }
  va_end(args);
  return cnt;
error:
  return -1;
}

int main(void) {
  unsigned int v = 42;
  myprintf("This integer is : %b\n", v);
  unsigned int v2 = 0;
  myprintf("That integer is : %b\n", v2);
}