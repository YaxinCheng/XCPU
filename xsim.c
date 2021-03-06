#include <stdio.h>
#include "xis.h"
#include "xcpu.h"
#include "xmem.h"
#include <stdlib.h>
#include <pthread.h>
#include "devices.h"

static void readXAS(const char*); 
static void* runCPU (void*);
static int cycles;

int main( int argc, char **argv ) {
  cycles = atoi(argv[1]);
  int cpu_num = argc < 4 ? 1 : atoi(argv[3]);
  xmem_init(65536);// init memory
  xcpu* cpu = (xcpu*)malloc(sizeof(xcpu) * cpu_num);// init cpu
  int i;
  for (i = 0; i < cpu_num; i+=1) {
    cpu[i].pc = 0;
    cpu[i].id = i;
    cpu[i].num = cpu_num;
  }
  readXAS(argv[2]);
  pthread_t *tid = (pthread_t*)malloc(sizeof(pthread_t) * (cpu_num + 3));
  for (i = 0; i < cpu_num + 3; i += 1) {
    void*(*threadFunc)(void*) = NULL;
    void* args = NULL;
    if (i == 0) { threadFunc = device_display; }
    else if (i == 1) { threadFunc = device_keyboard; }
    else if (i == 2) { threadFunc = device_random; }
    else { 
      threadFunc = runCPU;
      args = cpu + (i - 3);
    }
    if(pthread_create(tid + i, NULL, threadFunc, args)) { perror("Threading"); }
  }
  for (i = 3; i < cpu_num + 3; i += 1) {
    pthread_join(tid[i], NULL);
  }
  return 0;
}

// Function: static void readXAS(const char* fileName)
//------------------------------------------------
// Read in binary data from the file
//
// fileName: a string of the file name needs to be read
//
// return: void
//
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

// Function: static void* runCPU(void* cpu_context)
//------------------------------------------------
// Worker function for threads
//
// cpu_context: xcpu pointer to a cpu
//
// return: 0
//
static void* runCPU(void* cpu_context) {
  int curr_cycle = 0;
  xcpu* cpu = (xcpu*)cpu_context;
  for(; curr_cycle < cycles || cycles == 0; curr_cycle += 1 ) {
    int status = xcpu_execute(cpu);
    if (status == 0) { perror("Illegal instruction executed"); }
    if (status == 2) { break; }// null reached
    if ((cpu->state & 0x0002) != 0) { xcpu_print(cpu); }
  }
  return 0;
}
