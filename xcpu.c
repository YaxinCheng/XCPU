#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "xis.h"
#include "xcpu.h"

#include "xmem.h"
/* Use 
 *   xcpu_print( c );
 * to print cpu state, where c is of type xcpu * 
 * See xcpu.h for full prototype of xcpu_print()
 */

extern int xcpu_execute( xcpu *c ) {

  /* Your code here */
  short ins, par; 
  {
    unsigned char instruction[2];
    xmem_load(c->pc, instruction);// Load command
    ins = instruction[0];
    par = instruction[1];
  }
  c->pc += 2;// Increase pc
  
  if (ins == I_BAD) { return 0; }
  short operand = ins >> 6; // Check the first two bits
  if (operand == 0) { // 00. The last 8 bits are 0
    if (ins == I_RET) { // ret
      unsigned char cmd[2];
      xmem_load(c->regs[X_STACK_REG], cmd);
      c->pc = (cmd[0] << 8) + cmd[1];
      c->regs[X_STACK_REG] += 2;
    } else if (ins == I_CLD) { c->state &= 0xFFFD; }// cld
    else if (ins == I_STD) { c->state |= 0x0002; }// std
  } else if (operand == 1) {// 01
    short param = par >> 4;
    if (ins == I_NEG) { c->regs[param] *= -1; }// neg
    else if (ins == I_NOT) { c->regs[param] = !c->regs[param]; }// not
    else if (ins == I_INC) { c->regs[param] += 1; }// inc
    else if (ins == I_DEC) { c->regs[param] -= 1; }// dec
    else if (ins == I_PUSH) { // push
      c->regs[X_STACK_REG] -= 2;
      unsigned char tmp[2] = {c->regs[param] >> 8, c->regs[param] & 255};
      xmem_store(tmp, c->regs[X_STACK_REG]);
    }
    else if (ins == I_POP) { // pop
      unsigned char tmp[2];
      xmem_load(c->regs[X_STACK_REG], tmp);
      c->regs[param] = (tmp[0] << 8) + tmp[1];
      c->regs[X_STACK_REG] += 2;
    }
    else if (ins == I_JMPR) { c->pc = c->regs[param]; }// jmpr
    else if (ins == I_CALLR) { // callr
      c->regs[X_STACK_REG] -= 2;
      unsigned char tmp[2] = {c->pc >> 8, c->pc & 255};
      xmem_store(tmp, c->regs[X_STACK_REG]);
      c->pc = c->regs[param];
    }
    else if (ins == I_OUT) { printf("%c", c->regs[param]); }
    else if (ins == I_BR) {// br
      if ((c->state & 0x0001) == 0x001) { c->pc += par - 2; }
    } else if (ins == I_JR) { c->pc += par - 2; }// jr
  } else if (operand == 2) {// 10
    short first = par >> 4;
    short second = par & 15;// par & 0000 1111
    if (ins == I_ADD) { c->regs[second] += c->regs[first]; }
    else if (ins == I_SUB) { c->regs[second] -= c->regs[first]; }
    else if (ins == I_MUL) { c->regs[second] *= c->regs[first]; }
    else if (ins == I_DIV) { c->regs[second] /= c->regs[first]; }
    else if (ins == I_AND) { c->regs[second] &= c->regs[first]; }
    else if (ins == I_OR ) { c->regs[second] |= c->regs[first]; }
    else if (ins == I_XOR) { c->regs[second] ^= c->regs[first]; }
    else if (ins == I_SHR) { c->regs[second] >>= c->regs[first]; }
    else if (ins == I_SHL) { c->regs[second] <<= c->regs[first]; }
    else if (ins == I_TEST || ins == I_CMP || ins == I_EQU) { 
      unsigned short rS1 = c->regs[first];
      unsigned short rS2 = c->regs[second];
      if ((ins == I_TEST && (rS1 & rS2))
          || (ins == I_CMP && (rS1 < rS2))
          || (ins == I_EQU && (rS1 == rS2))) { c->state |= 0x0001; }
      else { c->state &= 0xFFFE; }
    } else if (ins == I_MOV) { c->regs[second] = c->regs[first]; }
    else if (ins == I_LOAD) { 
      unsigned char tmp[2];
      xmem_load(c->regs[first], tmp); 
      c->regs[second] = (tmp[0] << 8) + tmp[1];
    }
    else if (ins == I_STOR) { 
      unsigned char tmp[2] = {c->regs[second] >> 8, c->regs[second] & 255 };
      xmem_store(tmp, c->regs[first]);
    }
    else if (ins == I_LOADB) {
      unsigned char instruction[2];
      xmem_load(c->regs[first], instruction);
      c->regs[second] = instruction[0];
    }
    else if (ins == I_STORB) {
      unsigned char instruction[2];
      xmem_load(c->regs[second], instruction);
      instruction[0] = c->regs[first];
      xmem_store(instruction, c->regs[second]);
    }
  } else { // 11
    short third_bit = (ins >> 5) & 1;// right shift 5, check the right most
    if (third_bit == 0) {
      unsigned char tmp[2];
      xmem_load(c->pc, tmp);
      unsigned short L = (tmp[0] << 8) + tmp[1];
      c->pc += 2;
      if (ins == I_JMP) { c->pc = L; }// JMP
      else if (ins == I_CALL) {// CALLR
        c->regs[X_STACK_REG] -= 2;
        unsigned char tmp[2] = {c->pc >> 8, c->pc & 255 };
        xmem_store(tmp, c->regs[X_STACK_REG]);
        c->pc = L;
      }
    } else {// LOADI
      unsigned char cmd[2];
      xmem_load(c->pc, cmd);
      c->regs[par>>4] = (cmd[0] << 8) + cmd[1];
      c->pc += 2;
    }
  }
  return 1; /* replace this as needed */
}


/* Not needed for assignment 1 */
int xcpu_exception( xcpu *c, unsigned int ex ) {
  return 0;
}
