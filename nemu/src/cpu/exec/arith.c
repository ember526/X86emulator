#include "cpu/exec.h"

make_EHelper(add) {
  //opcode : 
  //0x01=>Ev,Gv
  //0x03=>Gv,Ev 
  int dest_olds  = id_dest->val >> (id_dest->width*8 - 1); 
  int  src_s  = id_src->val >> (id_src->width*8 - 1);

  if(decoding.opcode==0x83)
    rtl_sext(&id_src->val, &id_src->val, id_src->width); 

  rtl_add(&id_dest->val, &id_dest->val, &id_src->val);
  operand_write(id_dest, &id_dest->val);

  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  rtlreg_t tcf =  (dest_olds && src_s && cpu.SF)  || ((dest_olds ^ src_s) ^ cpu.SF); //111
  rtlreg_t tof = (dest_olds && src_s && !cpu.SF)  || (!dest_olds && !src_s && cpu.SF); //110 001
  rtl_set_CF(&tcf);
  rtl_set_OF(&tof);
  print_asm_template2(add);
}

make_EHelper(sub) {
  //opcode : 
  //0x29  Ev<-Gv
  //0x2b  Gv<-Ev
  rtlreg_t tcf = (unsigned)id_dest->val < (unsigned)id_src->val;
  int dest_s  = id_dest->val >> (id_dest->width*8 - 1); 
  int  src_s  = id_src->val >> (id_src->width*8 - 1); 
  if(decoding.opcode==0x83)
    rtl_sext(&id_src->val, &id_src->val, id_src->width);
  rtl_sub(&id_dest->val, &id_dest->val, &id_src->val);
  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  rtlreg_t tof = (!dest_s && src_s && cpu.SF) || (dest_s && !src_s && !cpu.SF); //011 100
  rtl_set_CF(&tcf);
  rtl_set_OF(&tof);
  operand_write(id_dest, &id_dest->val);
 // }
  //else
  //  TODO();
  //printf("after sub dest_var %x / s-imm %d\n", id_dest->val, id_src->simm);
  print_asm_template2(sub);
}

make_EHelper(cmp) {
  //opcode : 
  //0x39   Ev <- Gv
  //0x3c   al <- Ib
  //0x3d   eAX <- Iv
  //0x38   Eb <- Gb 
  if(decoding.opcode==0x83)
    rtl_sext(&id_src->val, &id_src->val, id_src->width); 
  rtlreg_t tcf = (unsigned)id_dest->val < (unsigned)id_src->val;
  int dest_s  = id_dest->val >> (id_dest->width*8 - 1); 
  int  src_s  = id_src->val >> (id_src->width*8 - 1); 
  rtl_sub(&id_dest->val, &id_dest->val, &id_src->val);
  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  rtlreg_t tof = (!dest_s && src_s && cpu.SF) || (dest_s && !src_s && !cpu.SF); //011 100
  rtl_set_CF(&tcf);
  rtl_set_OF(&tof);
  //Log("Attention!%d!%d!%d!", id_src->val, id_dest->val, cpu.ZF);
  
  print_asm_template2(cmp);
}

make_EHelper(inc) {
  //opcode : 
  //0xfe  Eb
  //0x48-0x4f +rb/w
  //0xff  Ev
  rtlreg_t tcf = (unsigned)id_dest->val < (unsigned)id_src->val;
  int dest_s  = id_dest->val >> (id_dest->width*8 - 1); 
  int  src_s  = id_src->val >> (id_src->width*8 - 1); 
  
  rtlreg_t t0 = 1;
  rtl_add(&id_dest->val, &id_dest->val, &t0);
  rtl_update_ZFSF(&id_dest->val, id_dest->width);

  rtlreg_t tof = (!dest_s && src_s && cpu.SF) || (dest_s && !src_s && !cpu.SF); //011 100
  rtl_set_CF(&tcf);
  rtl_set_OF(&tof);
  operand_write(id_dest, &id_dest->val);
}

make_EHelper(dec) {
  //opcode : 
  //0xfe  Eb
  //0x40-0x47 +rb/w
  rtlreg_t tcf = (unsigned)id_dest->val < (unsigned)id_src->val;
  int dest_s  = id_dest->val >> (id_dest->width*8 - 1); 
  int  src_s  = id_src->val >> (id_src->width*8 - 1); 
  
  rtlreg_t t0 = 1;
  rtl_sub(&id_dest->val, &id_dest->val, &t0);
  rtl_update_ZFSF(&id_dest->val, id_dest->width);

  rtlreg_t tof = (!dest_s && src_s && cpu.SF) || (dest_s && !src_s && !cpu.SF); //011 100
  rtl_set_CF(&tcf);
  rtl_set_OF(&tof);
  operand_write(id_dest, &id_dest->val);

  print_asm_template1(dec);
}

make_EHelper(neg) {

  rtlreg_t t0  = 0;
  rtlreg_t tcf = 0 != id_dest;
  int dest_s  = id_dest->val >> (id_dest->width*8 - 1); 

  rtl_sub(&id_dest->val, &t0, &id_dest->val);
  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  rtlreg_t tof = !(dest_s ^ cpu.SF); //11
  rtl_set_CF(&tcf);
  rtl_set_OF(&tof);
  operand_write(id_dest, &id_dest->val);

  print_asm_template1(neg);
}

make_EHelper(adc) {
  rtl_add(&t2, &id_dest->val, &id_src->val);
  rtl_setrelop(RELOP_LTU, &t3, &t2, &id_dest->val);
  rtl_get_CF(&t1);
  rtl_add(&t2, &t2, &t1);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_setrelop(RELOP_LTU, &t0, &t2, &id_dest->val);
  rtl_or(&t0, &t3, &t0);
  rtl_set_CF(&t0);

  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_not(&t0, &t0);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template2(adc);
}

make_EHelper(sbb) {
  rtl_sub(&t2, &id_dest->val, &id_src->val);
  rtl_setrelop(RELOP_LTU, &t3, &id_dest->val, &t2);
  rtl_get_CF(&t1);
  rtl_sub(&t2, &t2, &t1);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_setrelop(RELOP_LTU, &t0, &id_dest->val, &t2);
  rtl_or(&t0, &t3, &t0);
  rtl_set_CF(&t0);

  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template2(sbb);
}

make_EHelper(mul) {
  rtl_lr(&t0, R_EAX, id_dest->width);
  rtl_mul_lo(&t1, &id_dest->val, &t0);

  switch (id_dest->width) {
    case 1:
      rtl_sr(R_AX, &t1, 2);
      break;
    case 2:
      rtl_sr(R_AX, &t1, 2);
      rtl_shri(&t1, &t1, 16);
      rtl_sr(R_DX, &t1, 2);
      break;
    case 4:
      rtl_mul_hi(&t2, &id_dest->val, &t0);
      rtl_sr(R_EDX, &t2, 4);
      rtl_sr(R_EAX, &t1, 4);
      break;
    default: assert(0);
  }

  print_asm_template1(mul);
}

// imul with one operand
make_EHelper(imul1) {
  //opcode : 
  //0xf7  eax
  rtl_lr(&t0, R_EAX, id_dest->width);
  rtl_imul_lo(&t1, &id_dest->val, &t0);

  switch (id_dest->width) {
    case 1:
      rtl_sr(R_AX, &t1, 2);
      break;
    case 2:
      rtl_sr(R_AX, &t1, 2);
      rtl_shri(&t1, &t1, 16);
      rtl_sr(R_DX, &t1, 2);
      break;
    case 4:
      rtl_imul_hi(&t2, &id_dest->val, &t0);
      rtl_sr(R_EDX, &t2, 4);
      rtl_sr(R_EAX, &t1, 4);
      break;
    default: assert(0);
  }

  print_asm_template1(imul);
}

// imul with two operands
make_EHelper(imul2) {
  //opcode : 
  //0x0f+0xaf  Gv <- Ev
  rtl_sext(&t0, &id_src->val, id_src->width);
  rtl_sext(&t1, &id_dest->val, id_dest->width);

  rtl_imul_lo(&t2, &t1, &t0);
  operand_write(id_dest, &t2);

  print_asm_template2(imul);
}

// imul with three operands
make_EHelper(imul3) {
  rtl_sext(&t0, &id_src->val, id_src->width);
  rtl_sext(&t1, &id_src2->val, id_src->width);
  rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);

  rtl_imul_lo(&t2, &t1, &t0);
  operand_write(id_dest, &t2);

  print_asm_template3(imul);
}

make_EHelper(div) {
  switch (id_dest->width) {
    case 1:
      rtl_lr(&t0, R_AX, 2);
      rtl_div_q(&t2, &t0, &id_dest->val);
      rtl_div_r(&t3, &t0, &id_dest->val);
      rtl_sr(R_AL, &t2, 1);
      rtl_sr(R_AH, &t3, 1);
      break;
    case 2:
      rtl_lr(&t0, R_AX, 2);
      rtl_lr(&t1, R_DX, 2);
      rtl_shli(&t1, &t1, 16);
      rtl_or(&t0, &t0, &t1);
      rtl_div_q(&t2, &t0, &id_dest->val);
      rtl_div_r(&t3, &t0, &id_dest->val);
      rtl_sr(R_AX, &t2, 2);
      rtl_sr(R_DX, &t3, 2);
      break;
    case 4:
      rtl_lr(&t0, R_EAX, 4);
      rtl_lr(&t1, R_EDX, 4);
      rtl_div64_q(&cpu.eax, &t1, &t0, &id_dest->val);
      rtl_div64_r(&cpu.edx, &t1, &t0, &id_dest->val);
      break;
    default: assert(0);
  }

  print_asm_template1(div);
}

make_EHelper(idiv) {
  switch (id_dest->width) {
    case 1:
      rtl_lr(&t0, R_AX, 2);
      rtl_idiv_q(&t2, &t0, &id_dest->val);
      rtl_idiv_r(&t3, &t0, &id_dest->val);
      rtl_sr(R_AL, &t2, 1);
      rtl_sr(R_AH, &t3, 1);
      break;
    case 2:
      rtl_lr(&t0, R_AX, 2);
      rtl_lr(&t1, R_DX, 2);
      rtl_shli(&t1, &t1, 16);
      rtl_or(&t0, &t0, &t1);
      rtl_idiv_q(&t2, &t0, &id_dest->val);
      rtl_idiv_r(&t3, &t0, &id_dest->val);
      rtl_sr(R_AX, &t2, 2);
      rtl_sr(R_DX, &t3, 2);
      break;
    case 4:
      rtl_lr(&t0, R_EAX, 4);
      rtl_lr(&t1, R_EDX, 4);
      rtl_idiv64_q(&cpu.eax, &t1, &t0, &id_dest->val);
      rtl_idiv64_r(&cpu.edx, &t1, &t0, &id_dest->val);
      break;
    default: assert(0);
  }

  print_asm_template1(idiv);
}
