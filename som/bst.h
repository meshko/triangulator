#ifndef _BST_H_
#define _BST_H_

#include "geom.h"

typedef struct _bstnode
{
  edge *edg;
  struct _bstnode *left, *right;
} bstnode;

bstnode* bst_create(edge *edg, vertex *);

void bst_insert(edge *, vertex *, bstnode **, double y);
void bst_remove(edge *, bstnode **, double y);
bstnode* bst_find(double, bstnode *, double y);
void bst_find_left(double, bstnode *, edge **, double y);
void bst_print(bstnode *);
void bst_free(bstnode *);

#endif
