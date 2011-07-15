#ifndef _PQUEUE_H_
#define _PQUEUE_H_

#include "geom.h"

typedef struct _pqueue
{
  vertex** elems;
  int size;
  int n;
} pqueue;

pqueue* pq_create(int size);
void pq_free(pqueue*);
int is_leaf(int, pqueue*);
int left_child(int, pqueue*);
int right_child(int, pqueue*);
int parent(int, pqueue*);
void pq_insert(vertex*, pqueue*);
vertex* pq_remove(pqueue*);
void siftdown(int, pqueue*);

#endif
