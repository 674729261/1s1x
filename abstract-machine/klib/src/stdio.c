#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <limits.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) { panic("Not implemented"); }

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {

  va_list argp;
  va_start(argp, fmt);
  for (int i = 0; fmt[i] != '\0'; i++) {
    if (fmt[i] == '%') {
      i++;
      switch (fmt[i]) {
      case '\0':
        goto error;
        break;

      case 'd': {
        int value = va_arg(argp, int);
        if (value == INT_MIN) {
          const char *int_min_decimal = "-2147483648";
          for (int j = 0; int_min_decimal[j]; j++) {
            *out = int_min_decimal[j];
            out++;
          }
        } else if (value == 0) {
          *out = '0';
          out++;
        } else {
          if (value < 0) {
            *out = '-';
            out++;
            value = -value;
          }
          char *begin = out;
          while (value) {
            *out = '0' + value % 10;
            out++;
            value /= 10;
          }
          size_t len = out - begin;
          for (int j = 0; j < len / 2; j++) {
            char tmp = begin[j];
            begin[j] = begin[len - 1 - j];
            begin[len - 1 - j] = tmp;
          }
        }
      } break;

      case 's': {
        char *str = va_arg(argp, char *);
        for (int j = 0; str[j]; j++) {
          *out = str[j];
          out++;
        }
      } break;

      default:
        goto error;
      }
    } else {
      *out = fmt[i];
      out++;
    }
  }

  va_end(argp);
  *out = '\0';
  return 0;

error:
  va_end(argp);
  return -1;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
