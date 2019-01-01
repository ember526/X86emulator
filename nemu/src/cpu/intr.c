#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
	rtl_push(&cpu.eflags);
	uint32_t CS = cpu.CS;
	cpu.IF = 0;
	rtl_push(&CS);
	rtl_push(&ret_addr);
	//printf("%d %d\n", NO, cpu.idtr.limit);
	assert(NO < cpu.idtr.limit);
	t0 = cpu.idtr.base + NO * 8; 
	t1 = vaddr_read(t0, 2);
	t2 = t1 + (vaddr_read(t0 + 4, 4) & 0xffff0000); 
	//Log("base         	0x%x", cpu.idtr.base);
	//Log("limit		  	0x%x", cpu.idtr.limit);
	//Log("NO		  	  	0x%x", NO);
	//Log("base address 	0x%x", t0);
	//Log("Offset Low   	0x%x", t1);
	//Log("Final Address    0x%x", t2);
	rtl_j(t2);
}

void dev_raise_intr() {
	cpu.intr = true;
}
