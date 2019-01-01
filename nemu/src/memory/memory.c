#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define PGSHFT    12      // log2(PGSIZE)
#define PTXSHFT   12      // Offset of PTX in a linear address
#define PDXSHFT   22      // Offset of PDX in a linear address
#define PDX(va)     (((uint32_t)(va) >> PDXSHFT) & 0x3ff)
#define PTX(va)     (((uint32_t)(va) >> PTXSHFT) & 0x3ff)
#define OFF(va)     ((uint32_t)(va) & 0xfff)
#define PAGE_SIZE         4096

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
	int map_NO = is_mmio(addr);
	if(map_NO == -1)	//memory
  		return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  	else {
  		uint32_t data = mmio_read(addr, len, map_NO);
  		//Log("Reading 0x%x from addr 0x%x in divice\n", data, addr);
  		return data;
  	}
}

void paddr_write(paddr_t addr, uint32_t data, int len) {
	int map_NO = is_mmio(addr);
	if(map_NO == -1)	//memory
  		memcpy(guest_to_host(addr), &data, len);
  	else {
  		///Log("Writing data 0x%x to addr 0x%x in divice\n",data, addr);
  		mmio_write(addr, len, data, map_NO);
  	}
  	return;
}

//uint32_t vaddr_read(vaddr_t addr, int len) {
//  return paddr_read(addr, len);
//}
//void vaddr_write(vaddr_t addr, uint32_t data, int len) {
//  paddr_write(addr, data, len);
//}
paddr_t page_translate(vaddr_t va) {
    //if(cpu.cr0.paging) {
    //    Log("cr0 : 0x%x", cpu.cr0.val);
    //    assert(0);
    //}
    if(!(/*cpu.cr0.protect_enable &&*/ cpu.cr0.paging)) {
        //Log("Vme not activated : %d", cpu.cr0.val);
        return va;
    }

    uint32_t pde = paddr_read((cpu.cr3 & ~0xfff) | (PDX(va)<<2), sizeof(uint32_t));
    if((pde & 0x1) == 0) { Log("wrong entry : 0x%x cr3:0x%x", pde, cpu.cr3); assert(0);}
    uint32_t pte = paddr_read((pde & ~0xfff) | (PTX(va)<<2), sizeof(uint32_t));
    if((pte & 0x1) == 0) { 
      //Log("wrong page table entry : 0x%x (vaddr : 0x%x / pde : 0x%x / cr3 : 0x%x)", pte, va , pde, cpu.cr3);
      //Log("VADDR      0x%x", va);
      //Log("cr3        0x%x", cpu.cr3);
      //Log("ADDR(pde)  0x%x", (cpu.cr3 & ~0xfff) | (PDX(va)<<2));
      //Log("pde        0x%x", pde);
      //Log("ADDR(pte)  0x%x", (pde & ~0xfff) | (PTX(va)<<2));
      //Log("pte        0x%x", pte);
      //Log("PADDR      0x%x", (pte & ~0xfff) | OFF(va));
      //Log("EIP        0x%x", cpu.eip);
      assert(0);
    }
    return ((pte & ~0xfff) | OFF(va));
}

uint32_t vaddr_read(vaddr_t addr, int len) {
    paddr_t paddr = page_translate(addr);
    if (OFF(addr) + len > PAGE_SIZE) {
        /* this is a special case, you can handle it later. */
        int len_this = PAGE_SIZE - OFF(addr);
        uint32_t data_this = paddr_read(paddr, len_this);
        int len_next = len - len_this;
        addr += len_this;
        uint32_t data_next = vaddr_read(addr, len_next);
        Log("Cross pages");
        return data_this | (data_next << (len_this * 8));
        //Log("%d %d %d", OFF(addr), len, PAGE_SIZE);
        //assert(0);
    }
    else {
        return paddr_read(paddr, len);
    }
}


void vaddr_write(vaddr_t addr, uint32_t data, int len) {
    if (OFF(addr) + len > PAGE_SIZE) {
        /* this is a special case, you can handle it later. */
        assert(0);
    }
    else {
        paddr_t paddr = page_translate(addr);
        paddr_write(paddr, data, len);
    }
}
