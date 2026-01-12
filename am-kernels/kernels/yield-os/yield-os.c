#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#define STACK_SIZE (4096 * 8)
typedef union {
  uint8_t stack[STACK_SIZE];
  struct {
    Context *cp;
  };
} PCB;
static PCB pcb[2], pcb_boot, *current = &pcb_boot;
int c = 0;
static void f(void *arg) {

  while (1) {
    c++;
    putch("?AB"[(uintptr_t)arg > 2 ? 0 : (uintptr_t)arg]);
	putch('\n');
    for (int volatile i = 0; i < 10000; i++)
      ;
    if (c == 50)
      halt(0);
    yield();
  }
}

static Context *schedule(Event ev, Context *prev) {
  current->cp = prev;
  current = (current == &(pcb[0]) ? &(pcb[1]) : &(pcb[0]));

  return current->cp;
}

int main() {
  cte_init(schedule);
  pcb[0].cp = kcontext((Area){pcb[0].stack, &pcb[0] + 1}, f, (void *)1L);
  pcb[1].cp = kcontext((Area){pcb[1].stack, &pcb[1] + 1}, f, (void *)2L);
  // printf("%08x!!\n", (uint32_t)&(pcb[0].cp));
  // printf("%08x!!\n", (uint32_t)&(pcb[1].cp));
  // printf("%08x??\n", (uint32_t)&(pcb[0].stack));
  // printf("%08x??\n", (uint32_t)&(pcb[1].stack));
  // printf("%08x??\n", (uint32_t)sizeof(pcb[0].stack));
  // printf("%08x##\n", (uint32_t)sizeof(Context));
  yield();
  panic("Should not reach here!");
}
