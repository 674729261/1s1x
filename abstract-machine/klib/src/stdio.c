#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <limits.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static int __format__(char *out, char *(update_funt)(char *, char),
                      const char *fmt, va_list argp) {

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
            out = update_funt(out, int_min_decimal[j]);
          }
        } else if (value == 0) {
          out = update_funt(out, '0');
        } else {
          if (value < 0) {
            out = update_funt(out, '-');
            value = -value;
          }
          char *begin = out;
          while (value) {
            out = update_funt(out, '0' + value % 10);
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
          out = update_funt(out, str[j]);
        }
      } break;

      default:
        goto error;
      }
    } else {
      out = update_funt(out, fmt[i]);
    }
  }

  out = update_funt(out, '\0');
  return 0;

error:
  va_end(argp);
  return -1;
}

static char *update_to_str(char *addr, char c) {
  *addr = c;
  return addr + 1;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  return __format__(out, update_to_str, fmt, argp);
  va_end(argp);
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

static char *update_to_serial(char *addr, char c) {
  *(volatile char *)addr = c;
  return addr;
}

#if defined(__ARCH_X86_NEMU)
#define DEVICE_BASE 0x0
#else
#define DEVICE_BASE 0xa0000000
#endif

#define MMIO_BASE 0xa0000000

#define SERIAL_PORT (DEVICE_BASE + 0x00003f8)

int printf(const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  return __format__((char *)SERIAL_PORT, update_to_serial, fmt, argp);
  va_end(argp);
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
