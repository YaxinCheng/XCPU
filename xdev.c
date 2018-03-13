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
  if (queue->count >= MAX_QUEUE) { return 0; }
  pthread_mutex_lock(&queue->lock);
  queue->_content[queue->_entry] = data;
  queue->_entry = (queue->_entry + 1) % MAX_QUEUE;
  queue->count += 1;
  pthread_mutex_unlock(&queue->lock);
  return 1;
}

static unsigned short queue_dequeue(Queue* queue) {
  if(queue->count == 0) { return EMP_QUEUE; } // flag for empty
  pthread_mutex_lock(&queue->lock);
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
  if (port >= NUM_PORTS || monitor.ports[port] != NULL) { return 0; }
  pthread_mutex_lock(&monitor.lock);
  monitor.ports[port] = (Queue*)malloc(sizeof(Queue) * 2);
  int i = 0;
  for(; i < 2; i += 1) { queue_init(&monitor.ports[port][i]); }
  pthread_mutex_unlock(&monitor.lock);
  return 1; 
}

int xdev_dev_put( unsigned short data, unsigned short port ) {
  if (port >= NUM_PORTS || monitor.ports[port] == NULL) { return 0; }
  if (!queue_enqueue(&monitor.ports[port][INPUT], data)) { return 0; }
  while(queue_peak(&monitor.ports[port][INPUT]) == data) { }
  return 1; 
}


int xdev_dev_get( unsigned short port, unsigned short *data ) {
  if (port >= NUM_PORTS || monitor.ports[port] == NULL) { return 0; }
  while((*data = queue_dequeue(&monitor.ports[port][OUTPUT])) == EMP_QUEUE) { }
  return 1;
}


int xdev_outp_sync( unsigned short data, unsigned short port ) {
  if (port >= NUM_PORTS || monitor.ports[port] == NULL) { return 0; }
  if (!queue_enqueue(&monitor.ports[port][OUTPUT], data)) { return 0; }
  while(queue_peak(&monitor.ports[port][OUTPUT]) == data) {}
  return 1; 
}


int xdev_outp_async( unsigned short data, unsigned short port ) {
  if (port >= NUM_PORTS || monitor.ports[port] == NULL) { return 0; }
  if (!queue_enqueue(&monitor.ports[port][OUTPUT], data)) { return 0; }
  return 1;
}


int xdev_inp_sync( unsigned short port, unsigned short *data ) {
  if (port >= NUM_PORTS || monitor.ports[port] == NULL) { return 0; }
  while ((*data = queue_dequeue(&monitor.ports[port][INPUT])) == EMP_QUEUE) { }
  return 1;
}


int xdev_inp_poll( unsigned short port, unsigned short *data ) {
  if (port >= NUM_PORTS || monitor.ports[port] == NULL) { return 0; }
  return (*data = queue_dequeue(&monitor.ports[port][INPUT])) != EMP_QUEUE;
}


