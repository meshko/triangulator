#include <stdlib.h>

#include "stack.h"

stack* init_stack(int size)
{
  stack* result;
  
  result = (stack*)malloc(sizeof(stack));
  result->array = malloc(sizeof(stack*)*size);
  result->top = 0;

  return result;
}

void free_stack(stack* stk)
{
  free(stk->array);
  free(stk);
}

vertex* pop(stack* stk) 
{
  if( stk->top == 0 ) 
    return NULL;
  else
    return stk->array[--(stk->top)];
}

vertex* peek(stack* stk) 
{
  if( stk->top == 0 ) 
    return NULL;
  else
    return stk->array[(stk->top)-1];
}

void push(vertex *v, stack *stk)
{
  stk->array[(stk->top)++] = v;
}
