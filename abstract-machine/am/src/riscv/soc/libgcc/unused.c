#include <am.h>
#include <klib-macros.h>

double __attribute__((section(".bootloader"))) __muldf3(double a, double b) {
  panic("Not implement");
}
long __attribute__((section(".bootloader"))) __fixdfdi(double a) {
  panic("Not implement");
}
