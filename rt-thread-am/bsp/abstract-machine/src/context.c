#include "rtdef.h"
#include <am.h>
#include <klib.h>
#include <rtthread.h>
#include <stdint.h>

static Context *ev_handler(Event e, Context *c) {
  switch (e.event) {
  case EVENT_YIELD: {
    rt_thread_t self = rt_thread_self();
    if (self->from_context_p)
      *(Context **)self->from_context_p = c;
    c = *(Context **)self->to_context_p;
  } break;
  case EVENT_IRQ_TIMER:
    break;
  case EVENT_IRQ_IODEV:
    break;

  default:
    printf("Unhandled event ID = %d\n", e.event);
    assert(0);
  }
  return c;
}

void __am_cte_init() { cte_init(ev_handler); }

void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to) {
  rt_thread_t self = rt_thread_self();
  self->from_context_p = from;
  self->to_context_p = to;
  yield();
}

void rt_hw_context_switch_to(rt_ubase_t to) { rt_hw_context_switch(0, to); }

void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from,
                                    rt_ubase_t to,
                                    struct rt_thread *to_thread) {
  assert(0);
}

typedef struct {
  void *arg;
  void (*tentry)(void *);
  void (*texit)(void);
} wrapped_parameter;

void wrapped_entry(void *param) {
  wrapped_parameter param_wrapped = *(wrapped_parameter *)param;
  param_wrapped.tentry(param_wrapped.arg);
  param_wrapped.texit();
}
rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter,
                             rt_uint8_t *stack_addr, void *texit) {
  stack_addr = (rt_uint8_t *)((uintptr_t)stack_addr & ~(sizeof(uintptr_t) - 1));
  // Context *context_addr = (Context *)(stack_addr - sizeof(Context));
  // context_addr->mepc = (uintptr_t)tentry - 4;
  // context_addr->mstatus = 0x1800;
  // context_addr->gpr[1] = (uintptr_t)texit;
  // context_addr->gpr[10] = (uintptr_t)parameter;

  Area stk = {.end = stack_addr};

  wrapped_parameter *stk_ext =
      (wrapped_parameter *)(stack_addr - sizeof(Context) -
                            sizeof(wrapped_parameter));
  stk_ext->texit = texit;
  stk_ext->tentry = tentry;
  stk_ext->arg = parameter;
  Context *context_addr = kcontext(stk, wrapped_entry, stk_ext);
  return (rt_uint8_t *)context_addr;
}
