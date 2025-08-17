#include <am.h>
#include <klib-macros.h>
#include <stdatomic.h>

bool mpe_init(void (*entry)()) {
  entry();
  panic("MPE entry returns");
}

int cpu_count() { return 1; }

int cpu_current() { return 0; }

int atomic_xchg(atomic_int *addr, atomic_int newval) {
  return atomic_exchange(addr, newval);
}
