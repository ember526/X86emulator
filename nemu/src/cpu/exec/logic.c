#include "cpu/exec.h"
#include "cpu/cc.h"

make_EHelper(test) {
  //opcode
  //0x84  Eb <- Gb
  //0xf7  E => Ib/Iv
  rtlreg_t t0 = 0;
  rtl_and(&id_dest->val, &id_dest->val, &id_src->val);
  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  print_asm_template2(test);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);
}

make_EHelper(and) {
  //printf("decoding.opcode:0x%x\n", decoding.opcode);
  rtlreg_t t0 = 0;
  if(decoding.opcode==0x83) {
    rtl_sext(&id_src->val, &id_src->val, id_src->width);
    //printf("after extension:0x%x\n", id_src->val);
  }
  rtl_and(&id_dest->val, &id_dest->val, &id_src->val);
  operand_write(id_dest, &id_dest->val);

  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);
  print_asm_template2(and);
}

make_EHelper(xor) {
  //opcode
  //0x33  Gv <- Ev
  rtlreg_t t0 = 0;
  if(decoding.opcode==0x83)
    rtl_sext(&id_src->val, &id_src->val, id_src->width);
  rtl_xor(&id_dest->val, &id_dest->val, &id_src->val);
  operand_write(id_dest, &id_dest->val);
  print_asm_template2(xor);

  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);
}

make_EHelper(or) {
  //opcode
  //0x09  Ev <- Gv
  //0x0a  Gb <- Eb
  rtlreg_t t0 = 0;
  rtl_or(&id_dest->val, &id_dest->val, &id_src->val);
  operand_write(id_dest, &id_dest->val);

  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);

  print_asm_template2(or);
}

make_EHelper(sar) {
  //opcode
  //0xc1  Ev <- Iv 
  rtl_sar(&id_dest->val, &id_dest->val, &id_src->val);
  // unnecessary to update CF and OF in NEMU
  operand_write(id_dest, &id_dest->val);

  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  print_asm_template2(sar);
}

make_EHelper(shl) {
  //opcode
  //0xd3  Ev <- CL 
  rtl_shl(&id_dest->val, &id_dest->val, &id_src->val);
  operand_write(id_dest, &id_dest->val);
  // unnecessary to update CF and OF in NEMU
  rtl_update_ZFSF(&id_dest->val, id_dest->width);

  print_asm_template2(shl);
}

make_EHelper(shr) {
  //opcode
  //0xc1  Ev <- Iv 
  //0xd3  Ev <- CL 
  //#ifdef DEBUG
  //Log("A possible mistake in the manuel at 0xc1 : shr");
  //#endif
  rtl_shr(&id_dest->val, &id_dest->val, &id_src->val);
  operand_write(id_dest, &id_dest->val);
  // unnecessary to update CF and OF in NEMU
  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint32_t cc = decoding.opcode & 0xf;

  rtl_setcc(&t2, cc);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(cc), id_dest->str);
}

make_EHelper(not) {
  //opcode
  //0xf7  Ev
  rtl_not(&id_dest->val, &id_dest->val);
  operand_write(id_dest, &id_dest->val);
  print_asm_template1(not);
}

make_EHelper(rol) {
  int temp = id_src->val;
  rtlreg_t tmpcf = 0, OF = 0;
  while(temp != 0) {
    tmpcf = id_dest->val >> (id_dest->width*8 - 1);
    id_dest->val = id_dest->val * 2 + tmpcf;
    temp = temp - 1;
  }
  operand_write(id_dest, &id_dest->val);

  OF = (id_dest->val >> (id_dest->width*8 - 1)) != tmpcf ? 1 : 0;
  rtl_set_CF(&tmpcf);
  rtl_set_OF(&OF);
}