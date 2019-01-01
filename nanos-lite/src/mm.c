#include "memory.h"
#include "proc.h"
static void *pf = NULL;

void* new_page(size_t nr_page) {
  void *p = pf;
  pf += PGSIZE * nr_page;
  memset(p, 0, PGSIZE * nr_page);
  assert(pf < (void *)_heap.end);
  return p;
}

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t new_brk) {
/*	if(current->max_brk == 0) {
		current->max_brk = current->cur_brk = heap_end;
		Log("mm_brk 0x%x 0x%x", new_brk, current->max_brk);
	}*/
	while(new_brk > current->max_brk) {
	/*Log("mm_brk 0x%x 0x%x", new_brk, current->max_brk);*/
		void *newpg = new_page(1);
		if(_map(&current->as, (void *)current->max_brk, newpg, 1) < 0)
			pf -= PGSIZE;
		current->max_brk += PGSIZE;
	}
	// we do not free memory, so the new_brk will always be larger
	current->cur_brk = new_brk;
  	return 0;
}

void init_mm() {
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from 0x%x", pf);

  _vme_init(new_page, free_page);
}
