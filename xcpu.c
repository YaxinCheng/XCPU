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

// Function: static unsigned short bytes2short(const unsigned char bytes[2])
// ----------------------------------------------------------------------
// Convert two bytes to a short number
//
// bytes: the value needs to be converted
//
// returns: the converted short number
//
static unsigned short bytes2short(const unsigned char bytes[2]) {
  return (bytes[0] << 8) + bytes[1];
}

extern int xcpu_execute( xcpu *c ) {

  /* Your code here */
  short ins, par; // Every word is [ ins(8-bit) | par (8-bit) ]
  {// read in the instruction and separate to ins and par
    unsigned char instruction[2];
    xmem_load(c->pc, instruction);// Load command
    ins = instruction[0];
    par = instruction[1];
  }
  c->pc += 2;// Increase pc
  
  if (ins == I_BAD) { return 2; }// null reached
  short operand = ins >> 6; // Check the first two bits
  if (operand == 0) { // 00
    if (ins == I_RET) { // ret
      unsigned char cmd[2];
      xmem_load(c->regs[X_STACK_REG], cmd);
      c->pc = bytes2short(cmd);
      c->regs[X_STACK_REG] += 2;
    } else if (ins == I_CLD) { c->state &= 0xFFFD; }// cld
    else if (ins == I_STD) { c->state |= 0x0002; }// std
    return 1;
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
      unsigned char cmd[2];
      xmem_load(c->regs[X_STACK_REG], cmd);
      c->regs[param] = bytes2short(cmd);
      c->regs[X_STACK_REG] += 2;
    }
    else if (ins == I_JMPR) { c->pc = c->regs[param]; }// jmpr
    else if (ins == I_CALLR) { // callr
      c->regs[X_STACK_REG] -= 2;
      unsigned char tmp[2] = {c->pc >> 8, c->pc & 255};
      xmem_store(tmp, c->regs[X_STACK_REG]);
      c->pc = c->regs[param];
    }
    else if (ins == I_OUT) { printf("%c", c->regs[param]); }// out
    else if (ins == I_BR) {// br
      if ((c->state & 0x0001) == 0x001) {
        par = par >> 7 ? -(256 - par) : par;// in br, the L is signed
        c->pc += par - 2; 
      }
    } else if (ins == I_JR) { // jr
      par = par >> 7 ? -(256 - par) : par;// in jr, the L is signed
      c->pc += par - 2;
    } else if (ins == I_CPUID) {// cpu id
      unsigned char tmp[2] = {c->id >> 8, c->id & 255};
      xmem_store(tmp, param);
    } else if (ins == I_CPUNUM) {// cpu num
      unsigned char tmp[2] = {c->num >> 8, c->num & 255};
      xmem_store(tmp, param);
    }
    return 1;
  } else if (operand == 2) {// 10
    short first = par >> 4;
    short second = par & 15;// par & 0000 1111
    if (ins == I_ADD) { c->regs[second] += c->regs[first]; }// add
    else if (ins == I_SUB) { c->regs[second] -= c->regs[first]; }//sub
    else if (ins == I_MUL) { c->regs[second] *= c->regs[first]; }//mul
    else if (ins == I_DIV) { c->regs[second] /= c->regs[first]; }//div
    else if (ins == I_AND) { c->regs[second] &= c->regs[first]; }//and
    else if (ins == I_OR ) { c->regs[second] |= c->regs[first]; }//or
    else if (ins == I_XOR) { c->regs[second] ^= c->regs[first]; }//xor
    else if (ins == I_SHR) { c->regs[second] >>= c->regs[first]; }//shr
    else if (ins == I_SHL) { c->regs[second] <<= c->regs[first]; }//shl
    else if (ins == I_TEST || ins == I_CMP || ins == I_EQU) {
      // test, cmp, equ
      unsigned short rS1 = c->regs[first];
      unsigned short rS2 = c->regs[second];
      if ((ins == I_TEST && (rS1 & rS2))
          || (ins == I_CMP && (rS1 < rS2))
          || (ins == I_EQU && (rS1 == rS2))) { 
        c->state |= 0x0001; 
      }
      else { c->state &= 0xFFFE; }
    } else if (ins == I_MOV) { c->regs[second] = c->regs[first]; }//mov
    else if (ins == I_LOAD) { //load
      unsigned char cmd[2];
      xmem_load(c->regs[first], cmd); 
      c->regs[second] = bytes2short(cmd);
    }
    else if (ins == I_STOR) { //stor
      unsigned char tmp[2] = {c->regs[first] >> 8, c->regs[first] & 255 };
      xmem_store(tmp, c->regs[second]);
    }
    else if (ins == I_LOADB) {//loadb
      unsigned char instruction[2];
      xmem_load(c->regs[first], instruction);
      c->regs[second] = instruction[0];
    }
    else if (ins == I_STORB) {//storb
      unsigned char instruction[2];
      xmem_load(c->regs[second], instruction);
      instruction[0] = c->regs[first];
      xmem_store(instruction, c->regs[second]);
    } else if (ins == I_LOADA) {//loada
      unsigned char bytes[2];
      xmem_load(c->regs[first], bytes);
      c->regs[second] = bytes2short(bytes);
    } else if (ins == I_STORA) {//stora
      unsigned char bytes[] = {c->regs[first] >> 8, c->regs[first] & 255};
      xmem_store(bytes, c->regs[second]);
    } else if (ins == I_SWAP) {//swap
      unsigned short v = c->regs[first];
      unsigned char bytes[2];
      xmem_load(c->regs[second], bytes);
      c->regs[first] = bytes2short(bytes);
      unsigned char tmp[] = {v >> 8, v & 255};
      xmem_store(tmp, c->regs[second]);
    }
    return 1;
  } else { // 11
    short third_bit = (ins >> 5) & 1;// right shift 5, check the right most
    if (third_bit == 0) {
      unsigned char cmd[2];
      xmem_load(c->pc, cmd);
      unsigned short L = bytes2short(cmd);
      if (ins == I_JMP) { c->pc = L; }// JMP
      else if (ins == I_CALL) {// CALL
        c->regs[X_STACK_REG] -= 2;
        unsigned char tmp[2] = {c->pc >> 8, c->pc & 255 };
        xmem_store(tmp, c->regs[X_STACK_REG]);
        c->pc = L;
      }
    } else {// LOADI
      unsigned char cmd[2];
      xmem_load(c->pc, cmd);
      c->regs[par>>4] = bytes2short(cmd);
      c->pc += 2;
    }
    return 1;
  }
  return 0; /* replace this as needed */
}


/* Not needed for assignment 1 */
int xcpu_exception( xcpu *c, unsigned int ex ) {
  return 0;
}
