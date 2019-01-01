#include "cpu/exec.h"
#include "device/port-io.h"
void difftest_skip_ref();
void difftest_skip_dut();

make_EHelper(lidt) {
  cpu.idtr.limit = vaddr_read(id_dest->reg, 2);
  cpu.idtr.base = vaddr_read(id_dest->reg + 2, 4);
  //printf("%x addr0x%x\n", cpu.eip, cpu.idtr.limit);
  if (decoding.is_operand_size_16) {
    cpu.idtr.base &= 0x00ffffff;
  }

  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  if(id_dest->reg == 0) {
    cpu.cr0.val = id_src->val;
  }
  else {
    assert(id_dest->reg == 3);
    cpu.cr3 = id_src->val;
  }
  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  assert(id_src->reg == 0 || id_src->reg == 3);
  t0 = id_src->reg == 0 ? cpu.cr0.val : cpu.cr3;
  rtl_sr(id_dest->reg, &t0, 4); 
  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}

make_EHelper(int) {
  raise_intr(id_dest->imm, decoding.seq_eip);
  print_asm("int %s", id_dest->str);

#if defined(DIFF_TEST) && defined(DIFF_TEST_QEMU)
  difftest_skip_dut();
#endif
}

make_EHelper(iret) {
  rtl_pop(&t0);
  rtl_pop(&t1);
  cpu.CS = t1;
  rtl_pop(&cpu.eflags);
  rtl_j(t0);
  print_asm("iret");
}

make_EHelper(in) {
  switch (decoding.opcode) {
    case 0xec : id_dest->val = pio_read_b(id_src->val); break;
    case 0xed : if(decoding.is_operand_size_16)TODO(); else id_dest->val = pio_read_l(id_src->val); break;
    default :TODO();
  }
  operand_write(id_dest, &id_dest->val);
  print_asm_template2(in);

#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}

make_EHelper(out) {
  switch (decoding.opcode) {
    case 0xee : pio_write_b(id_dest->val, id_src->val); break;
    default :TODO();
  }
  

  print_asm_template2(out);

#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}
