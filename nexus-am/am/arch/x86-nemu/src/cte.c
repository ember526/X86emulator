#include <am.h>
#include <x86.h>
#include "klib.h"
static _Context* (*user_handler)(_Event, _Context*) = NULL;

void vectrap();
void vecnull();
void vecsys();
void irq0();
extern void get_cur_as(_Context *c);
extern void _switch(_Context *c);
_Context* irq_handle(_Context *tf) {
  get_cur_as(tf);
  _Context *next = tf;
  //printf("0x%d\n", tf->irq);
  if (user_handler) {
    _Event ev = {0};
    switch (tf->irq) {
      case 32  :ev.event = _EVENT_IRQ_TIMER; break;
      case 0x80:ev.event = _EVENT_SYSCALL; break;
      case 0x81:ev.event = _EVENT_YIELD; break;
      default: ev.event = _EVENT_ERROR; break;
    }

    next = user_handler(ev, tf);
    if (next == NULL) {
      next = tf;
    }
  }
  //printf("irq--->0x%x\n", (uintptr_t)next);
  _switch(next);
  return next;
}

static GateDesc idt[NR_IRQ];

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  // initialize IDT
  for (unsigned int i = 0; i < NR_IRQ; i ++) {
    idt[i] = GATE(STS_TG32, KSEL(SEG_KCODE), vecnull, DPL_KERN);
  }

  // -------------------- system call --------------------------
  idt[0x81] = GATE(STS_TG32, KSEL(SEG_KCODE), vectrap, DPL_KERN);
  idt[0x80] = GATE(STS_TG32, KSEL(SEG_KCODE), vecsys, DPL_KERN);
  idt[32] = GATE(STS_TG32, KSEL(SEG_KCODE), irq0, DPL_KERN);
  set_idt(idt, sizeof(idt));

  // register event handler
  user_handler = handler;

  return 0;
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
  _Context *c = (_Context*)stack.end - 1;
  //stack.end -= sizeof(_Context);
  c->cs = 8;
  c->eip = (uintptr_t)entry;
  printf("Kernel context : start(0x%x)->end(0x%x).\n", stack.start, stack.end);
  return c;
}

void _yield() {
  asm volatile("int $0x81");
}

int _intr_read() {
  return 0;
}

void _intr_write(int enable) {
}
