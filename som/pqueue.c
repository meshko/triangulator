/*
  vertex priority queue
*/

#include <stdlib.h>

#include "pqueue.h"

static int greater();
static void swap();

pqueue* pq_create(int size)
{
  pqueue* result;
  result = (pqueue*)malloc(sizeof(pqueue));
  result->size=size;
  result->elems = malloc(sizeof(void*)*size);
  result->n = 0;
  return result;
}

void pq_free(pqueue* pq)
{
  free(pq->elems);
  free(pq);
}

int is_leaf(int pos, pqueue* pq)
{
  return (pos >= pq->n/2) && (pos < pq->n);
}

int left_child(int pos, pqueue* pq)
{
  return 2*pos + 1;
}

int right_child(int pos, pqueue* pq)
{
  return 2*pos + 2;
}

int parent(int pos, pqueue* pq)
{
  return (pos-1)/2;
}

void pq_insert(vertex* v, pqueue* pq)
{
  int curr = pq->n++;
  pq->elems[curr] = v;                 /* Start at end of heap */
  /* Now sift up until curr's parent's key > curr's key */
  while ((curr!=0) && greater(pq->elems[curr], pq->elems[parent(curr,pq)]) )
    {
      swap(curr, parent(curr, pq), pq);
      curr = parent(curr, pq);
    }
}

vertex* pq_remove(pqueue* pq)
{
  if( pq->n == 0 ) return NULL;
  
  pq->n--;
  swap(0, pq->n, pq);
  if (pq->n != 0)          /* Not on last element */
    siftdown(0, pq);       /* Put new heap root val in correct place */
  return pq->elems[pq->n];
}

void siftdown(int pos, pqueue* pq)
{
  int j;
  
 while (!is_leaf(pos, pq)) 
    {
      j = left_child(pos, pq);
      if ( (j < (pq->n-1)) && (greater(pq->elems[j+1], pq->elems[j])) ) 
	j++; /* j is now index of child with greater value */
     
      if ( !greater(pq->elems[j], pq->elems[pos]) )
	return;

      swap(pos, j, pq);
      pos = j;  /* Move down */
    }
}

static void swap(int i, int j, pqueue* pq)
{
  vertex* t;
  t = pq->elems[i];
  pq->elems[i] = pq->elems[j];
  pq->elems[j] = t;
}

static int greater(vertex* v1, vertex* v2)
{
  return (v1->y > v2->y) || ((v1->y == v2->y) && (v1->x < v2->x));
}
