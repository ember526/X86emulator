#include "common.h"
#include "syscall.h"
#include "fs.h"
#include "proc.h"
int sys_exit(uintptr_t no) {
	_halt(no);
	return 0;
}

int sys_yield() {
	_yield();
	return 0;
}

int sys_write(int fd, void *buf, size_t count){
  	return fs_write(fd, buf, count);
}
extern int mm_brk(uintptr_t new_brk, uintptr_t heap_end);
int sys_brk(void *addr, void *heap_end) {
	//program_break = (uintptr_t)addr;
	return mm_brk((uintptr_t)addr, (uintptr_t)heap_end);
}
extern void naive_uload (PCB *pcb, const char *filename);
_Context* do_syscall(_Context *c) {

  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  switch (a[0]) {
  	case SYS_exit  			: /*naive_uload(NULL, "/bin/init"); break;//*/sys_exit(a[1]);  break;
  	case SYS_yield 			: c->GPRx = sys_yield();    break;
  	case SYS_open			: c->GPRx = fs_open((const char *)a[1], 0, 0);  break;	
  	case SYS_read			: c->GPRx = fs_read(a[1], (void *)a[2], a[3]); 	break;
  	case SYS_write			: c->GPRx = sys_write(a[1], (void *)a[2], a[3]);	break;
  	case SYS_kill			: panic("Unhandled syscall ID = %d", a[0]);	break;
  	case SYS_getpid			: panic("Unhandled syscall ID = %d", a[0]);	break;
  	case SYS_close			: c->GPRx = fs_close(a[1]);	break;
  	case SYS_lseek			: c->GPRx = fs_lseek(a[1], a[2], a[3]);	break;
  	case SYS_brk			: c->GPRx = sys_brk((void *)a[1], (void *)a[1]);	break;
  	case SYS_fstat			: panic("Unhandled syscall ID = %d", a[0]);	break;
  	case SYS_time			: panic("Unhandled syscall ID = %d", a[0]);	break;
  	case SYS_signal			: panic("Unhandled syscall ID = %d", a[0]);	break;
  	case SYS_execve			:printf("=>%s\n", (const char *)a[1]); naive_uload (NULL, (const char *)a[1]);	break;
  	case SYS_fork			: panic("Unhandled syscall ID = %d", a[0]);	break;
  	case SYS_link			: panic("Unhandled syscall ID = %d", a[0]);	break;
  	case SYS_unlink			: panic("Unhandled syscall ID = %d", a[0]);	break;
  	case SYS_wait			: panic("Unhandled syscall ID = %d", a[0]);	break;
  	case SYS_times			: panic("Unhandled syscall ID = %d", a[0]);	break;
  	case SYS_gettimeofday	: panic("Unhandled syscall ID = %d", a[0]);	break; 
    default  : panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
