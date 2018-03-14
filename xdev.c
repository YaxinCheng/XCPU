#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "xdev.h"

#define NUM_PORTS 4
#define MAX_QUEUE 10
#define EMP_QUEUE 65535
#define INPUT 0
#define OUTPUT 1

typedef struct {
  unsigned short* _content;
  unsigned short _exit;
  unsigned short _entry;
  unsigned short count;
  pthread_mutex_t lock;
} Queue;

typedef struct {
  Queue* ports[NUM_PORTS]; 
  pthread_mutex_t lock;
} Monitor;

static void queue_init(Queue* q) {
  q->_content = (unsigned short*)malloc(sizeof(unsigned short) * MAX_QUEUE);
  q->_exit = 0;
  q->_entry = 0;
  q->count = 0;
  pthread_mutex_init(&q->lock, NULL);
}

static int queue_enqueue(Queue* queue, unsigned short data) {
  pthread_mutex_lock(&queue->lock);
  if (queue->count >= MAX_QUEUE) { 
    pthread_mutex_unlock(&queue->lock);
    return 0; 
  }
  queue->_content[queue->_entry] = data;
  queue->_entry = (queue->_entry + 1) % MAX_QUEUE;
  queue->count += 1;
  pthread_mutex_unlock(&queue->lock);
  return 1;
}

static unsigned short queue_dequeue(Queue* queue) {
  pthread_mutex_lock(&queue->lock);
  if(queue->count <= 0) { 
    pthread_mutex_unlock(&queue->lock);
    return EMP_QUEUE; 
  } 
  unsigned short data = queue->_content[queue->_exit];
  queue->_exit = (queue->_exit + 1) % MAX_QUEUE;
  queue->count -= 1;
  pthread_mutex_unlock(&queue->lock);
  return data;
}

static unsigned short queue_peak(Queue* queue) {
  return queue->count == 0 ? EMP_QUEUE : queue->_content[queue->_exit];
}

static Monitor monitor = { .ports = { NULL }, .lock=PTHREAD_MUTEX_INITIALIZER };

int xdev_associate_port( unsigned short port ) {
  pthread_mutex_lock(&monitor.lock);
  if (port >= NUM_PORTS || monitor.ports[port] != NULL) { 
    pthread_mutex_unlock(&monitor.lock);
    return 0; 
  }
  monitor.ports[port] = (Queue*)malloc(sizeof(Queue) * 2);
  int i = 0;
  for(; i < 2; i += 1) { queue_init(&monitor.ports[port][i]); }
  pthread_mutex_unlock(&monitor.lock);
  return 1; 
}

int xdev_dev_put( unsigned short data, unsigned short port ) {
  if (port >= NUM_PORTS || monitor.ports[port] == NULL) { return 0; }
  //printf(" %d >>> put is called\n", port);
  if (!queue_enqueue(&monitor.ports[port][INPUT], data)) { return 0; }
  while(queue_peak(&monitor.ports[port][INPUT]) == data) { }
  //printf(" %d >>> put is finished, data: %d\n", port, data);
  return 1; 
}


int xdev_dev_get( unsigned short port, unsigned short *data ) {
  if (port >= NUM_PORTS || monitor.ports[port] == NULL) { return 0; }
  //printf(" %d >>> get is called\n", port);
  while((*data = queue_dequeue(&monitor.ports[port][OUTPUT])) == EMP_QUEUE) { }
  //printf(" %d >>> get is finished, data: %d\n", port, *data);
  return 1;
}


int xdev_outp_sync( unsigned short data, unsigned short port ) {
  if (port >= NUM_PORTS || monitor.ports[port] == NULL) { return 0; }
  //printf(" %d >>> outp is called\n", port);
  if (!queue_enqueue(&monitor.ports[port][OUTPUT], data)) { return 0; }
  while(queue_peak(&monitor.ports[port][OUTPUT]) == data) {}
  //printf(" %d >>> outp is finished, data = %d\n", port, data);
  return 1; 
}


int xdev_outp_async( unsigned short data, unsigned short port ) {
  if (port >= NUM_PORTS || monitor.ports[port] == NULL) { return 0; }
  //printf(" %d >>> outp async is called, data = %d\n", port, data);
  if (!queue_enqueue(&monitor.ports[port][OUTPUT], data)) { return 0; }
  return 1;
}


int xdev_inp_sync( unsigned short port, unsigned short *data ) {
  if (port >= NUM_PORTS || monitor.ports[port] == NULL) { return 0; }
  //printf(" %d >>> inp sync is called\n", port);
  while ((*data = queue_dequeue(&monitor.ports[port][INPUT])) == EMP_QUEUE) { }
  //printf(" %d >>> inp sync is finished, data = %d \n", port, *data);
  return 1;
}


int xdev_inp_poll( unsigned short port, unsigned short *data ) {
  if (port >= NUM_PORTS || monitor.ports[port] == NULL) { return 0; }
  //printf(" %d >>> inp poll is called\n", port);
  return (*data = queue_dequeue(&monitor.ports[port][INPUT])) != EMP_QUEUE;
}


