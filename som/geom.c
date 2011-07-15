#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "geom.h"

vertex* vtx_create(double x, double y)
{
    vertex* result = (vertex*)malloc(sizeof(vertex));
    
    result->x = x;
    result->y = y;
    result->edg_next = result->edg_prev = NULL;
    result->diag = result->pdiag = NULL;
    result->type = VT_UNKNOWN;
    return result;
}

void vtx_print(vertex* vtx)
{
    printf("vertex(%f,%f)", vtx->x, vtx->y);
}

polygon* pol_create(vertex *v)
{
  polygon* result = (polygon*)malloc(sizeof(polygon));
  result->v = v;
  result->next = NULL;
  return result;
}

void pol_free(polygon *p)
{
  if(p->next != NULL) pol_free(p->next);
  free(p);
}

edge* edg_create(vertex* v1, vertex* v2)
{
    edge* result = (edge*)malloc(sizeof(edge));
    
    if(v1 == v2) 
      {
	printf("ATTEMPT TO CREATE 0-EDGE!!!\n");
	return NULL;
      }

    result->v1 = v1;
    result->v2 = v2;
    result->helper = NULL;

    v1->edg_next = result;
    v2->edg_prev = result;
    return result;
}

void edg_print(edge* edg)
{
    printf("edge: ");
    vtx_print(edg->v1);
    printf(" ");
    vtx_print(edg->v2);
    printf("\n");
}

double edg_startx(edge *edg)
{
  return edg->v1->x;
}

double vtx_angle(vertex *vtx)
{
  return edg_angle(vtx->edg_prev, vtx->edg_next);
}

/* 
   determine chain on which vertix lies. only for vertices of monotone 
   polygon.
   1: left
   0: right
*/
int check_chain(vertex *vtx)
{
  return VRT_NEXT((*vtx))->y < VRT_PREV((*vtx))->y;
}

int diag_exist(vertex *v1, vertex *v2, vertex *v3)
{
  double vect1[3], vect2[3], vect3[3];
  int chain;

  if( v1 == NULL || v2 == NULL || v3 == NULL )
    return 0;

  vect1[0] = v2->x - v1->x; vect1[1] =  v2->y - v1->y; vect1[2] = 0;
  vect2[0] = v3->x - v2->x; vect2[1] =  v3->y - v2->y; vect2[2] = 0;

  cross_prod(vect1,vect2,vect3);
  chain = check_chain(v2);
  printf("%d %f\n", chain, vect3[2]);
  return ((vect3[2] > 0.0 && chain == 1) || (vect3[2] < 0.0 && chain == 0) );
}

double edg_angle(edge *edg1, edge *edg2)
{
  double v1[] = {edg1->v2->x - edg1->v1->x, edg1->v2->y - edg1->v1->y, 0};
  double v2[] = {edg2->v2->x - edg2->v1->x, edg2->v2->y - edg2->v1->y, 0};
  double v3[3];
  double cp, angle;
  
  cross_prod(v1,v2,v3);
  cp = sqrt(v3[0]*v3[0]+v3[1]*v3[1]+v3[2]*v3[2]);

  angle =  asin(cp/(edg_len(edg1)*edg_len(edg2)));
  if(v3[2] < 0) angle = 2*M_PI - angle;
  return angle;
}

double edg_len(edge *edg)
{
  return sqrt((edg->v1->x - edg->v2->x)*(edg->v1->x - edg->v2->x)+
	      (edg->v1->y - edg->v2->y)*(edg->v1->y - edg->v2->y));
}

double dot_prod(double v1[3], double v2[3])
{
  int i;
  double r = 0;
  for(i = 0; i < 3; i++)
    r+=(v2[i]*v1[i]);

  return r;
}

void cross_prod(double v1[3], double v2[3], double res[3])
{
  res[0] = v1[1]*v2[2]-v1[2]*v2[1];
  res[1] = v1[2]*v2[0]-v1[0]*v2[2];
  res[2] = v1[0]*v2[1]-v1[1]*v2[0];
}

static int vtx_below(vertex v1, vertex v2)
{
  return v1.y < v2.y || (v1.y == v2.y && v1.x > v2.x);
}

static int vtx_above(vertex v1, vertex v2)
{
  return v1.y > v2.y || (v1.y == v2.y && v1.x < v2.x);
}

vtype vtx_type(vertex *vtx)
{
  vertex *vn, *vp;
  double angle;
  
  if( vtx == NULL ) return VT_UNKNOWN;

  if( vtx->type != VT_UNKNOWN ) 
    return vtx->type;

  angle = vtx_angle(vtx);
  vn = VRT_NEXT((*vtx)), vp = VRT_PREV((*vtx));

  if( vtx_below(*vn, *vtx) && vtx_below(*vp, *vtx) )
    if( angle < M_PI )
      vtx->type = VT_START;
    else
      vtx->type = VT_SPLIT;
  else
    if( vtx_above(*vn, *vtx) && vtx_above(*vp, *vtx) )
      if( angle < M_PI )
	vtx->type = VT_END;
      else
	vtx->type = VT_MERGE;
    else
      vtx->type = VT_REGULAR;
  
  return vtx->type;
}
