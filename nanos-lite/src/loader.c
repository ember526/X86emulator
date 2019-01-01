#include "proc.h"
#include "fs.h"
#define DEFAULT_ENTRY 0x8048000


static uintptr_t loader(PCB *pcb, const char *filename) {
  //TODO();
  //printf("%s\n", _heap.start);
  //size_t size = get_ramdisk_size();
  //printf("---->%d\n", size);
  int fd = fs_open(filename, 0 , 0);

  void *vaddr = (void *)DEFAULT_ENTRY;
  int nr_pages = fs_filesz(fd) / PGSIZE + (fs_filesz(fd) % PGSIZE ? 1 : 0);
  //Log("nr_pages(%d) %d", nr_pages, fs_filesz(fd) / PGSIZE);
  for (int i = 0; i < nr_pages; ++i)
  {
    void *paddr = new_page(1);
    _map(&pcb->as, vaddr+i*PGSIZE, paddr, 1);
    fs_read(fd, (uintptr_t *)paddr, PGSIZE);
    //if(len!=PGSIZE)
    //Log("loader %d %d %d",i , len, PGSIZE);
  }
  pcb->max_brk = DEFAULT_ENTRY + (nr_pages) * PGSIZE;

  fs_close(fd);
  //ramdisk_read((uintptr_t *)DEFAULT_ENTRY, 0, get_ramdisk_size());
  return DEFAULT_ENTRY;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void *entry) {
  // _protect(&pcb->as);
  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _kcontext(stack, entry, NULL);
}

void context_uload(PCB *pcb, const char *filename) {
  _protect(&pcb->as);
  uintptr_t entry = loader(pcb, filename);

  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}
