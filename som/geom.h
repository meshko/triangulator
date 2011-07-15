#ifndef _GEOM_H_
#define _GEOM_H_

/* Some <math.h> files do not define M_PI... */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef enum _vtype 
{VT_START = 0, VT_END, VT_REGULAR, VT_SPLIT, VT_MERGE, VT_UNKNOWN} vtype; 

typedef struct _vertex
{
  double x, y;
  struct _edge *edg_next, *edg_prev;
  struct _vertex *diag, *pdiag;
  vtype type;
} vertex;


#define VRT_NEXT(vrt) vrt.edg_next->v2
#define VRT_PREV(vrt) vrt.edg_prev->v1

vertex* vtx_create(double, double);
void vtx_print(vertex *);

typedef struct _edge
{
  struct _vertex *v1, *v2, *helper;
} edge;

typedef struct _polygon
{
  vertex *v;
  struct _polygon *next;
} polygon;

polygon* pol_create(vertex *);
void pol_free(polygon *);

edge* edg_create(vertex*, vertex*);
void edg_print(edge *);
double edg_startx(edge *);
double edg_angle(edge *, edge *);
double edg_len(edge *);

double vtx_angle(vertex *);
vtype vtx_type(vertex *);
int check_chain(vertex *);
int diag_exist(vertex *, vertex *, vertex *);

double dot_prod(double[3], double[3]);
void cross_prod(double[3], double[3], double[3]);

#endif
