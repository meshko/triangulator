#ifndef _STACK_H_
#define _STACIK_H_

#include "geom.h"

typedef struct _stack
{
  vertex** array;
  int top;
} stack;

stack* init_stack(int);
void free_stack(stack*);
vertex* pop(stack*);
vertex* peek(stack*);
void push(vertex*, stack*);

#endif
