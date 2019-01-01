#include "common.h"
extern _Context* do_syscall(_Context *c);
extern _Context* schedule(_Context *prev);
static _Context* do_event(_Event e, _Context* c) {
	//printf("----->%d\n", e.event);
  switch (e.event) {
  	case _EVENT_YIELD : /*printf("->yield\n");*/ return schedule(c); break;
  	case _EVENT_SYSCALL : do_syscall(c); break;
  	case _EVENT_IRQ_TIMER: /*Log("time intr");*/ _yield(); break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return NULL;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  _cte_init(do_event);
}
