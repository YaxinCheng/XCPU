#include <stdio.h>
#include "xis.h"
#include "xcpu.h"
#include "xmem.h"
#include <stdlib.h>

static void readXAS(const char*); 

int main( int argc, char **argv ) {
  int cycles = atoi(argv[1]);
  xmem_init(65536);// init memory
  xcpu* cpu = (xcpu*)malloc(sizeof(xcpu));// init cpu
  cpu->pc = 0;
  readXAS(argv[2]);
  int curr_cycle = 0;
  for(; curr_cycle < cycles; curr_cycle += 1 ) {
    int running = xcpu_execute(cpu);
    if (running == 0) {
      printf("Illegal instruction;");
      return 1;
    }
    if (running == 2) { break; }
    if ((cpu->state & 0x0002) != 0) 
      xcpu_print(cpu);
  }
  return 0;
}

static void readXAS(const char* fileName) {
  FILE* xasFile = fopen(fileName, "rb");
  if (xasFile == NULL) { perror("Error openning file"); }
  unsigned char ins[2];
  unsigned int index = 0;
  while (fread(ins, 2, 1, xasFile)) {
    xmem_store(ins, index);
    index += 2;
  }
  fclose(xasFile);  
}

