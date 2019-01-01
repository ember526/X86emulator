#include "proc.h"

#define MAX_NR_PROC 4
extern void naive_uload(PCB *pcb, const char *filename);
static PCB pcb[MAX_NR_PROC] __attribute__((used));
static PCB pcb_boot;
PCB *current;
int fg_pcb = 1;
void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite for the %dth time!", j);
    j ++;
    _yield();
  }
}

void init_proc() {
	//before PA4 : naive_uload(NULL, "/bin/init");
	//context_uload(&pcb[1], "/bin/dummy");
	context_uload(&pcb[0], "/bin/hello");
	context_uload(&pcb[1], "/bin/pal");
	context_uload(&pcb[2], "/bin/pal");
	context_uload(&pcb[3], "/bin/events");
	//context_kload(&pcb[1], (void *)hello_fun);
  	switch_boot_pcb();
}

_Context* schedule(_Context *prev) {
	// save the context pointer
	current->cp = prev;
	static int n = 0;
	if(current == &pcb[fg_pcb] && n == 100) {
		current = &pcb[0];
		n = 0;
		//printf("Switching to pcb 1 : user context.\n");
	}
	else {
		current = &pcb[fg_pcb];
		n++;
		//printf("Switching to pcb 0 : kernel context.\n");
	}
	//current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
	// then return the new context
	return current->cp;
  	//return NULL;
}
