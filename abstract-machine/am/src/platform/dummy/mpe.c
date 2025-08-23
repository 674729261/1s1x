#include <am.h>
#include <stdatomic.h>

bool mpe_init(void (*entry)()) { return false; }

int cpu_count() { return 1; }

int cpu_current() { return 0; }

int atomic_xchg(atomic_int *addr, atomic_int newval) { return 0; }
