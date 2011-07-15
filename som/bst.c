/*
  Binary search tree for edges
*/

#include <stdlib.h>
#include <stdio.h>
#include <values.h>

#include "bst.h"
#include "geom.h"

static double calc_key(edge *, double);

bstnode* bst_create(edge *edg, vertex *hlp)
{
    bstnode *result = (bstnode*)malloc(sizeof(bstnode));
    
    result->left = result->right = NULL;
    result->edg = edg;
    edg->helper = hlp;
    return result;
}

void bst_free(bstnode *bst)
{
  if( bst == NULL ) return;

  bst_free(bst->left);
  bst_free(bst->right);
  free(bst);
}

void bst_insert(edge *edg, vertex *hlp, bstnode **p_tree, double y)
{
  double new_key, old_key;
  if (*p_tree == NULL) 
    {
      *p_tree = bst_create(edg, hlp);
      return;
    }

  old_key = calc_key((*p_tree)->edg, y);
  new_key = calc_key(edg, y);
  if (old_key > new_key)
    bst_insert(edg, hlp, &((*p_tree)->left), y);
  else
    bst_insert(edg, hlp, &((*p_tree)->right), y);
}

static edge* getmin(bstnode *tree) 
{
  if (tree->left == NULL)
    return tree->edg;
  else 
    return getmin(tree->left);
}

static void deletemin(bstnode **p_tree) 
{
  bstnode *tn;

  if ((*p_tree)->left == NULL)
    {
      tn = (*p_tree)->right;
      free(*p_tree);
      *p_tree = tn;
    }
  else 
    {
      deletemin(&(*p_tree)->left);
    }
}

void bst_remove(edge* edg, bstnode **p_tree, double y)
{
  double cur_key, del_key;
  edge *temp;
  bstnode *tnode;

  if (*p_tree == NULL) return;
  cur_key = calc_key((*p_tree)->edg, y); 
  del_key = calc_key(edg, y); 
  if (del_key < cur_key)
    bst_remove(edg, &((*p_tree)->left), y);
  else if (del_key > cur_key)
    bst_remove(edg, &((*p_tree)->right), y);
  else 
    { /* Found it */
      if ((*p_tree)->left == NULL)
	{
	  tnode = (*p_tree)->right;
	  free(*p_tree);
	  *p_tree = tnode;
	}
      else 
	if ((*p_tree)->right == NULL)
	  {
	    tnode = ((*p_tree)->left);
	    free(*p_tree);
	    *p_tree = tnode;
	  }
      else 
	{ /* Two children */
	  temp = getmin((*p_tree)->right);
	  (*p_tree)->edg = temp;
	  deletemin(&((*p_tree)->right));
	}
    }
}

/* this is *dumb* way to search bin tree. replace */
void bst_find_left(double key, bstnode *tree, edge **found, double y)
{
  int cur_key, best_key;
  
  if( tree == NULL ) return;
  
  best_key = (*found == NULL) ? MININT : calc_key((*found), y);
  cur_key = calc_key(tree->edg, y);
  if(cur_key < key && cur_key > best_key )
    *found = tree->edg;
    
  bst_find_left(key, tree->left, found, y);
  bst_find_left(key, tree->right, found, y);
}

bstnode* bst_find(double key, bstnode *tree, double y)
{
  printf("not implemented\n");
  /*double cur_key;
  if (tree == NULL) return NULL;

  cur_key = calc_key(p_tree->edg, y);
  if (cur_key > key) return bst_find(rt.left(), key);
  else if (it.key() == key)  return it;
  else  return findhelp(rt.right(), key);
  */
  return NULL;
}

void bst_print(bstnode *tree)
{
  if( tree == NULL ) return;
  edg_print(tree->edg);
  bst_print(tree->left);
  bst_print(tree->right);
}

static double calc_key(edge *edg, double y)
{
  double k = (edg->v2->y - edg->v1->y)/(edg->v2->x - edg->v1->x);

  return edg->v2->x == edg->v1->x ? 
    edg->v1->x : 
    (y - edg->v1->y + k*edg->v1->x)/k;
}
