/*
  cs120a CAD/CAM
  Final project: Polygon triangulation demo
  Written by Mikhail Kruk
  Professor Hod Lipson
  Brandeis University, Fall 2000
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <float.h>

#include <GL/glut.h>

#include "geom.h"
#include "pqueue.h"
#include "bst.h"
#include "stack.h"

/* window id, size */
int wndMain = -1, WIDTH = 20;

int startx, starty, moving = 0, angle_m = 0, angle2_m = 0;
int width = 480, height = 480;
int VTX_NUM = 0;
enum mode {ENTRY, VERTICES, MONOTONE, TRIANGULATE, DONE};
enum mode cur_mode = ENTRY;

/* vertex dimension unit or something */
#define VDU .2 

#define FALSE 0
#define TRUE !FALSE

vertex* vertices[1000];
vertex* diags[1000];
polygon **m_polygons;
int poly_count = 0, cur_pol; /* number of monotone polygons and current pol. */
int diag_count = 0; /* total number of diagonals to draw */
int pv_num; /* vertices in the current polygon */
int j;

/* some global data structures */
pqueue *pq;
bstnode *bst;
stack *stk;
vertex *vrt;

/* function prototypes */
void display( void );
void reshape(int, int);
void keyboard (unsigned char, int, int);
void keyboardS (int, int, int);
void exiter(int);

int triangulate();
int make_monotone();
void fix_direction();
void init_edges();
void init_triang();
double calc_area();
void add_diagonal(vertex *v1, vertex *v2, int new_p);

void draw_polygon(polygon*);
void draw_vertices();
void draw_vertex(vertex*);

void debug_dump();
void debug_clear();
void debug_print_chain(vertex*);

void reshape(int w, int h)
{
  glViewport(0, 0, w, h);
  width = w;
  height = h;
  glutPostRedisplay();
}

void process_diagonals(vertex* sv)
{
  polygon *cp = pol_create( sv ), *np, *sp = cp;
  vertex *cv = sv, *nv, *vn1, *vn2, *tv;
  edge* diag;

  do
    {
      nv = NULL;
      if( cv->diag != NULL )
	{
	  nv = cv->diag;
	  cv->diag->pdiag = NULL;
	  cv->diag = NULL;
	}
      else
	if( cv->pdiag != NULL )
	  {
	    nv = cv->pdiag;
	    cv->pdiag->diag = NULL;
	    cv->pdiag = NULL;
	  }
      
      if( nv == NULL )
	{
	  cv = VRT_NEXT((*cv));
	  np = pol_create(cv);
	  cp->next = np;
	  cp = np;
	}
      else
	{
	  vn1 = vtx_create(cv->x, cv->y);
	  vn2 = vtx_create(nv->x, nv->y);
	  diag = edg_create(vn1, vn2);
	  vn1->edg_prev = cv->edg_prev;
	  vn2->edg_next = nv->edg_next;
	  
	  vn1->edg_prev->v2 = vn1;
	  vn2->edg_next->v1 = vn1;
	  
	  edg_create(nv, cv);
	  
	  if(cv->pdiag != NULL)
	    {
	      tv = cv;
	      do
		{
		  tv = VRT_NEXT((*tv));
		}
	      while( tv != cv && tv != cv->pdiag );

	      if( tv == cv )
		{
		  vn1->pdiag = cv->pdiag;
		  cv->pdiag = NULL;
		  vn1->pdiag->diag = vn1;
		}
	    }
	  if(cv->diag != NULL)
	    {
	      tv = cv;
	      do
		{
		  tv = VRT_NEXT((*tv));
		}
	      while( tv != cv && tv != cv->diag );

	      if( tv == cv )
		{
		  vn1->diag = cv->diag;
		  cv->diag = NULL;
		  vn1->diag->pdiag = vn1;
		}
	    }
	  if(nv->pdiag != NULL)
	    {
	      tv = nv;
	      do
		{
		  tv = VRT_NEXT((*tv));
		}
	      while( tv != nv && tv != nv->pdiag );

	      if( tv == nv )
		{
		  vn2->pdiag = nv->pdiag;
		  nv->pdiag = NULL;
		  vn2->pdiag->diag = vn2;
		}
	    }
	  if(nv->diag != NULL)
	    {
	      tv = nv;
	      do
		{
		  tv = VRT_NEXT((*tv));
		}
	      while( tv != nv && tv != nv->diag );

	      if( tv == nv )
		{
		  vn2->diag = nv->diag;
		  nv->diag = NULL;
		  vn2->diag->pdiag = vn2;
		}
	    }
	  
	  process_diagonals(cv);
	  process_diagonals(vn1);
	  pol_free(sp);
	  return;
	}
    }
  while( cv != sv );

  m_polygons[cur_pol++] = sp;
}

void draw_polygon(polygon *pol)
{
  polygon *start = pol;

  glBegin(GL_LINE_STRIP);
  
  if(pol == NULL) 
    {
      printf("ERR: empty polygon!\n");
      debug_dump();
      exit(-1);
    }

  while(pol != NULL)
    {
      glVertex3f(pol->v->x,pol->v->y,0);

      pol = pol->next;
    }
  glVertex3f(start->v->x,start->v->y,0);
  glEnd();
}

void debug_print_chain(vertex* sv)
{
  vertex *cv = sv;

  do
    {
      vtx_print(cv);
      printf("\n");
      cv = VRT_NEXT((*cv));
    }
  while(cv != sv);
}

void debug_clear()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void debug_dump()
{
  int i;
  for(i = 0; i < VTX_NUM; i++)
    printf("vertices[%d] = vtx_create(%d,%d);\n",i,
	   (int)(vertices[i]->x), (int)(vertices[i]->y));
}

void draw_vertices()
{
  int i;
  
  glBegin(GL_LINE_STRIP);
  for(i = 0; i < VTX_NUM; i++)
    glVertex3f(vertices[i]->x, vertices[i]->y, 0);
  
  if( cur_mode != ENTRY )
    glVertex3f(vertices[0]->x, vertices[0]->y, 0);
  glEnd();
}

void draw_vertex(vertex *v)
{
  if( v != vrt )
    glColor3f(1.0,1.0,1.0);
  else
    glColor3f(0.0,1.0,0.0);

  switch( vtx_type(v) )
    {
    case VT_START:
      glBegin(GL_LINE_STRIP);
        glVertex3f(v->x-VDU, v->y-VDU,0.1);
        glVertex3f(v->x+VDU, v->y-VDU,0.1);
        glVertex3f(v->x+VDU, v->y+VDU,0.1);
	glVertex3f(v->x-VDU, v->y+VDU,0.1);
	glVertex3f(v->x-VDU, v->y-VDU,0.1);
      glEnd();
      break;
    case VT_END:
      glBegin(GL_QUADS);
        glVertex3f(v->x-VDU, v->y-VDU,0.1);
        glVertex3f(v->x+VDU, v->y-VDU,0.1);
        glVertex3f(v->x+VDU, v->y+VDU,0.1);
	glVertex3f(v->x-VDU, v->y+VDU,0.1);
      glEnd();
      break;
    case VT_REGULAR:  
      glBegin(GL_QUADS);
        glVertex3f(v->x-VDU*2, v->y,0.1);
        glVertex3f(v->x, v->y-VDU*2,0.1);
        glVertex3f(v->x+VDU*2, v->y,0.1);
	glVertex3f(v->x, v->y+VDU*2,0.1);
      glEnd();
      break;
    case VT_SPLIT:
      glBegin(GL_TRIANGLES);
        glVertex3f(v->x-VDU*2, v->y,0.1);
        glVertex3f(v->x+VDU*2, v->y,0.1);
	glVertex3f(v->x, v->y+VDU*2,0.1);
      glEnd();
      break;
    case VT_MERGE:
      glBegin(GL_TRIANGLES);
        glVertex3f(v->x-VDU*2, v->y,0.1);
        glVertex3f(v->x+VDU*2, v->y,0.1);
	glVertex3f(v->x, v->y-VDU*2,0.1);
      glEnd();
      break;
    default:
      printf("unknown vertex type!\n");
    }
}

void draw_diags()
{
  int i;
  glColor3f(.8f, 0.8f, 0.1f);
  glBegin(GL_LINES);
    for(i = 0; i < diag_count; i++)
      glVertex3f(diags[i]->x, diags[i]->y,0);
  glEnd();
}

void display( void )
{
  int i;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
  /* mouse view change, from some glut example */
  glPushMatrix();
  glRotatef(angle2_m, 1.0, 0.0, 0.0); 
  glRotatef(angle_m, 0.0, 1.0, 0.0);

  glColor3f(1.0, 1.0, 1.0);

  switch( cur_mode )
    {
    case ENTRY:
      draw_vertices();
      break;
    case VERTICES:
      draw_vertices();
      for(i = 0; i < VTX_NUM; i++)
	draw_vertex(vertices[i]);
      break;
    case MONOTONE:
      draw_vertices();
      for(i = 0; i < VTX_NUM; i++)
	draw_vertex(vertices[i]);
      
      if( vrt != NULL )
	{
	  glColor3f(0.6,.6,0.6);
	  glBegin(GL_LINES);
	    glVertex3f(-WIDTH, vrt->y, 0);
	    glVertex3f(WIDTH, vrt->y, 0);
	  glEnd();
	}
      draw_diags();
      break;
    case DONE:
    case TRIANGULATE:
      if( cur_mode != DONE )
	glColor3f(0.5,0.5,0.5);
      else
	glColor3f(1.0,1.0,1.);
      
      for(i = 0; i < poly_count; i++)
	draw_polygon(m_polygons[i]);
      draw_diags();

      if( cur_mode != DONE )
	{
	  glColor3f(1.0,1.0,1.0);
	  glLineWidth(2.0);
	  draw_polygon(m_polygons[cur_pol]);
	  glLineWidth(1.0);

	  glColor3f(0.0,0.0,1.0);
	  glTranslatef(peek(stk)->x, peek(stk)->y, 0);
	  glutSolidSphere(.3,5,5);
	  glTranslatef(-peek(stk)->x, -peek(stk)->y, 0);
	}
      break;    
    }
  
  glPopMatrix();

  glFlush();
  glutSwapBuffers();
}

void init( void )
{
  glClearColor(0.0, 0.0, 0.0, 0.0); /* backgound color */

  //glEnable(GL_LINE_SMOOTH); 
  glEnable(GL_DEPTH_TEST); 

  /* Setup the view of the cube. */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-WIDTH, WIDTH, -WIDTH, WIDTH, -WIDTH, WIDTH);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(.0, .0, 5.0,  /* eye is at */
    0.0, 0.0, 0.0,      /* center is at (0,0,0) */
    0.0, 1.0, 0.);      /* up is in positive Y direction */

}

void keyboard (unsigned char key, int x1, int y1)
{
   switch (key) 
     {
     case 27:
       exiter(0);
       break;
     case 32:
       switch( cur_mode )
	 {
	 case VERTICES:
	   cur_mode = MONOTONE;
	   glutPostRedisplay();
	   break;
	 case MONOTONE:
	   if( !make_monotone() )
	     {
	       pq_free(pq);
	       bst_free(bst);
	       poly_count++; /* always one more polygons than diagonals */

	       m_polygons = (polygon**)malloc(sizeof(polygon*)*poly_count);
	       cur_pol = 0;
	       /* actually split into monotone polygons */
	       process_diagonals(vertices[0]);

	       cur_pol = 0; 	       /* reuse some of the counters... */
	       diag_count = 0;
	       init_triang(); /* preapare first polygon */
	       cur_mode = TRIANGULATE;
	     }
	   glutPostRedisplay();
	   break;
	 case TRIANGULATE:
	   if( !triangulate() )
	     {
	       if( cur_pol < poly_count-1 )
		 {
		   cur_pol++;

		   pq_free(pq);
		   free_stack(stk);

		   init_triang();
		   glutPostRedisplay();
		 }
	       else
		 cur_mode = DONE;
	     }
	   glutPostRedisplay();
	   break;
	 default:
	   return;
	 }
       break;
     }
}

void keyboardS (int key, int x1, int y1)
{
   switch (key) 
     {
     default:
       return;
     }
}

/* from some glut example */
static void
mouse(int button, int state, int x, int y)
{
  if (button == GLUT_RIGHT_BUTTON) 
    {
      if (state == GLUT_DOWN) {
	moving = 1;
	startx = x;
	starty = y;
      }
      if (state == GLUT_UP) {
	moving = 0;
      }
      return;
    }
  if( button == GLUT_LEFT_BUTTON && state == GLUT_UP && cur_mode == ENTRY )
    {
      vertices[VTX_NUM++] = vtx_create( (x-width/2.0)*WIDTH/width*2.0, 
					(-(y-height/2.0)*WIDTH/height*2.0) );
      glutPostRedisplay();
      return;
    }
  if( button == GLUT_MIDDLE_BUTTON && state == GLUT_UP && cur_mode == ENTRY )
    {
      if( VTX_NUM == 0 )
	exit(0);
      fix_direction();
      init_edges();
      cur_mode = VERTICES;
      glutPostRedisplay();
      return;
    }
}

/* from some glut example */
static void
motion(int x, int y)
{
  if (moving) {
    angle_m = angle_m + (x - startx);
    angle2_m = angle2_m + (y - starty);
    startx = x;
    starty = y;
    glutPostRedisplay();
  }

}

void exiter(int res)
{
  exit(res);
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  glutInitWindowSize( width, height ); 
  wndMain = glutCreateWindow("Polygon Triangulation Demo");

  init();

  glutKeyboardFunc(keyboard);
  glutSpecialFunc(keyboardS);
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);

  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  
  glutMainLoop();
  return 0;
}

/* prepare to triangulate current polygon */
void init_triang()
{
  vertex *sv, *cv;

  sv = m_polygons[cur_pol]->v; cv = sv;
  pv_num = 0;
  pq = pq_create(VTX_NUM);
  do
    {
      pq_insert(cv, pq);
      cv = VRT_NEXT((*cv));
      pv_num++;
    }
  while(cv != sv );
  
  stk = init_stack(VTX_NUM);
  push(pq_remove(pq), stk);
  push(pq_remove(pq), stk);
  j = 3;
}

int triangulate()
{
  vertex *v, *vs, *vp;
  
  if(j++ < pv_num)
    {
      v = pq_remove(pq);
      
      if( check_chain(peek(stk)) != check_chain(v) )
	{
	  printf("diff chain\n");
	  vp = NULL;
	  while( (vs = pop(stk)) != NULL )
	    {
	      if( vp == NULL ) vp = vs;
	      if( peek(stk) != NULL )
		{
		  diags[diag_count++] = v;
		  diags[diag_count++] = vs;
		}
	    }

	  push(vp, stk);
	  push(v, stk);
	}
      else
	{
	  printf("same chain\n");
	  vp = pop(stk);
	  while( diag_exist(vp, v, peek(stk)) && (vs = pop(stk)) != NULL )
	    {
	      diags[diag_count++] = v;
	      diags[diag_count++] = vs;
	      glutPostRedisplay();
	      vp = vs;
	    }
	  push(vp,stk);
	  push(v,stk);
	}
      return TRUE;
    }
  else
    {
      pop(stk);
      v = pq_remove(pq);
      if( (vs = pop(stk)) != NULL && peek(stk) != NULL )
	{
	  diags[diag_count++] = v;
	  diags[diag_count++] = vs;
	  glutPostRedisplay();
	  return TRUE;
	}
      else
	return FALSE;
    }
}

/*
  adds diagonal to split polygon into monotone polygons
  also stored in the diags array for drawing purposes;
*/
void add_diagonal(vertex *v1, vertex *v2, int new_p)
{
  /*vertex *v1n, *v2n, *vt;
    edge *diag, *te;*/

  printf("insert diagonal: ");
  vtx_print(v1);
  vtx_print(v2);
  printf("\n");

  v1->diag = v2;
  v2->pdiag = v1;

  diags[diag_count++] = v1;
  diags[diag_count++] = v2;

  poly_count++;
}

double calc_area()
{
  int i;
  double s;
  for(i = 0, s = 0; i < VTX_NUM-1; i++)
    s+=((vertices[i+1]->x - vertices[i]->x)*
	(vertices[i+1]->y + vertices[i]->y)/2.0);
  s+=((vertices[0]->x - vertices[i]->x)*
	(vertices[0]->y + vertices[i]->y)/2.0);
  return s;
}

void fix_direction()
{
  double area;
  int i;
  vertex *cv;
  
  area = calc_area();
  printf("area: %f\n", area);
  if( area > 0 )
    {
      for(i = 0; i < VTX_NUM/2; i++)
	{
	  cv = vertices[i];
	  vertices[i] = vertices[VTX_NUM-i-1];
	  vertices[VTX_NUM-i-1] = cv;
	}
      printf("polygon inverted\n");
    }
}

/* create edges between vertices, put vertices into the priority queue*/
void init_edges()
{
  int i;

  pq = pq_create(VTX_NUM);
  bst = NULL;

  for(i = 0; i < VTX_NUM; i++)
    {
      pq_insert(vertices[i], pq);
      if(i > 0) 
	edg_create(vertices[i-1], vertices[i]);
    }
  edg_create(vertices[VTX_NUM-1], vertices[0]);
} 

/* 
   break polygon into simpler monotone polygons.
   this function processes one vertex from the priority queue at a time and 
   adds a diagonal if needed. Return TRUE if there are more vertices left,
   FALSE if finished processing
*/
int make_monotone()
{
  int i;
  edge *edge;

  vrt = pq_remove(pq);
  if( vrt == NULL ) return FALSE;

  switch( vtx_type(vrt) )
    {
    case VT_START:
      printf("\nstart vertex\n");
      bst_insert(vrt->edg_next, vrt, &bst, vrt->y);
      break;
    case VT_END:
      printf("\nend vertex\n");
      bst_remove(vrt->edg_prev, &bst, vrt->y);
      if( vtx_type(vrt->edg_prev->helper) == VT_MERGE )
	{
	  add_diagonal(vrt, vrt->edg_prev->helper, TRUE);
	}
      break;
    case VT_REGULAR:  
      printf("\nregular vertex\n");
      if( (VRT_NEXT((*vrt)))->y < (VRT_PREV((*vrt)))->y )
	{
	  bst_remove(vrt->edg_prev, &bst, vrt->y);
	  bst_insert(vrt->edg_next, vrt, &bst, vrt->y);
	  if( vtx_type(vrt->edg_prev->helper) == VT_MERGE )
	    { 
	      add_diagonal(vrt, vrt->edg_prev->helper, TRUE);
	    }
	}
      else
	{
	  edge = NULL;
	  bst_find_left(vrt->x, bst, &edge, vrt->y);
	  
	  if(edge == NULL) 
	    {
	      printf("ERR: edge not found!\n");
	      break;
	    }
	  
	  if( edge != NULL && vtx_type(edge->helper) == VT_MERGE )
	    {
	      add_diagonal(vrt, edge->helper, TRUE);
	    }
	  edge->helper = vrt;
	}
      break;
    case VT_SPLIT:
      printf("\nsplit vertex\n");
      edge = NULL;
      bst_find_left(vrt->x, bst, &edge, vrt->y);
      
      bst_insert(vrt->edg_next, vrt, &bst, vrt->y);
      
      if(edge == NULL) 
	{
	  printf("ERR: edge not found!\n");
	  break;
	}
      
      add_diagonal(vrt, edge->helper, TRUE);
      
      edge->helper = vrt;
      
      break;
    case VT_MERGE:
      printf("\nmerge vertex\n");
      bst_remove(vrt->edg_prev, &bst, vrt->y);
      if( vtx_type(vrt->edg_prev->helper) == VT_MERGE )
	{
	  add_diagonal(vrt, vrt->edg_prev->helper, TRUE);
	}
      
      edge = NULL;
      bst_find_left(vrt->x, bst, &edge, vrt->y);
      
      if(edge == NULL) 
	{
	  printf("ERR: edge not found!\n");
	  break;
	}
      
      if( edge != NULL && vtx_type(edge->helper) == VT_MERGE )
	{
	  add_diagonal(vrt, edge->helper, TRUE);
	}
      edge->helper = vrt;
      break;
    default:
      printf("unknown vertex type!\n");
    }
  return TRUE;
}

 /*vertices[0] = vtx_create(2,1);
  vertices[1] = vtx_create(19,5);
  vertices[2] = vtx_create(22,13);
  vertices[3] = vtx_create(18,10);
  vertices[4] = vtx_create(17,14);
  vertices[5] = vtx_create(14,10);
  vertices[6] = vtx_create(12,13);
  vertices[7] = vtx_create(10,9);
  vertices[8] = vtx_create(8,12);
  vertices[9] = vtx_create(6,8);
  vertices[10] = vtx_create(5,11);
  VTX_NUM = 11;*/

  /*vertices[0] = vtx_create(3,4);
  vertices[1] = vtx_create(2,12);
  vertices[2] = vtx_create(0,3);
  vertices[3] = vtx_create(-12,8);
  vertices[4] = vtx_create(-2,2);
  vertices[5] = vtx_create(-12,-3);
  vertices[6] = vtx_create(-1,0);
  vertices[7] = vtx_create(-6,-12);
  vertices[8] = vtx_create(1,-3);
  vertices[9] = vtx_create(5,-12);
  vertices[10] = vtx_create(5,-2);
  vertices[11] = vtx_create(16,-4);
  VTX_NUM = 12;*/

  /*vertices[0] = vtx_create(0,8);
  vertices[1] = vtx_create(-2,15);
  vertices[2] = vtx_create(-5,2);
  vertices[3] = vtx_create(-13,0);
  vertices[4] = vtx_create(-5,-1);
  vertices[5] = vtx_create(-14,-13);
  vertices[6] = vtx_create(0,-4);
  vertices[7] = vtx_create(-2,-15);
  vertices[8] = vtx_create(3,-4);
  vertices[9] = vtx_create(8,-14);
  vertices[10] = vtx_create(8,-3);
  vertices[11] = vtx_create(15,-7);
  vertices[12] = vtx_create(11,-1);
  vertices[13] = vtx_create(18,0);
  vertices[14] = vtx_create(9,1);
  vertices[15] = vtx_create(17,10);
  vertices[16] = vtx_create(10,5);
  vertices[17] = vtx_create(10,14);
  vertices[18] = vtx_create(3,7);
  vertices[19] = vtx_create(3,16);
  VTX_NUM = 20;*/
 
  /*vertices[0] = vtx_create(5,9);
  vertices[1] = vtx_create(3,10);
  vertices[2] = vtx_create(4,5);
  vertices[3] = vtx_create(1,1);
  vertices[4] = vtx_create(3,2);
  vertices[5] = vtx_create(6,4);
  vertices[6] = vtx_create(7,2);
  vertices[7] = vtx_create(10,1);
  vertices[8] = vtx_create(8,4);
  vertices[9] = vtx_create(11,2);
  vertices[10] = vtx_create(14,1);
  vertices[11] = vtx_create(9,5);
  vertices[12] = vtx_create(11,7);
  vertices[13] = vtx_create(9,8);
  vertices[14] = vtx_create(8,6);
  vertices[15] = vtx_create(8,9);
  vertices[16] = vtx_create(7,9);
  vertices[17] = vtx_create(6,6);
  VTX_NUM = 18;*/

  /*vertices[0] = vtx_create(4,12);
  vertices[1] = vtx_create(2,3);
  vertices[2] = vtx_create(7,2);
  vertices[3] = vtx_create(9,5);
  vertices[4] = vtx_create(6,7);
  vertices[5] = vtx_create(5,4);
  vertices[6] = vtx_create(7,5);
  vertices[7] = vtx_create(6,3);
  vertices[8] = vtx_create(3,4);
  vertices[9] = vtx_create(7,9);
  vertices[10] = vtx_create(15,9);
  vertices[11] = vtx_create(16,4);
  vertices[12] = vtx_create(14,3);
  vertices[13] = vtx_create(14,6);
  vertices[14] = vtx_create(15,5);
  vertices[15] = vtx_create(15,6);
  vertices[16] = vtx_create(13,8);
  vertices[17] = vtx_create(12,1);
  vertices[18] = vtx_create(18,5);
  vertices[19] = vtx_create(15,12);
  VTX_NUM = 20;*/

  /*
  vertices[0] = vtx_create(7,3);
  vertices[1] = vtx_create(8,10);
  vertices[2] = vtx_create(2,10);
  vertices[3] = vtx_create(3,2);
  vertices[4] = vtx_create(5,6);
  
  VTX_NUM = 5;*/
  
  /*vertices[0] = vtx_create(16,12);
  vertices[1] = vtx_create(9,11);
  vertices[2] = vtx_create(2,1);
  vertices[3] = vtx_create(11,6);
  vertices[4] = vtx_create(9,2);
  vertices[5] = vtx_create(13,5);
  vertices[6] = vtx_create(14,3);

  VTX_NUM = 7;*/

  /*vertices[0] = vtx_create(14,10.4);
  vertices[1] = vtx_create(7.4,8);
  vertices[2] = vtx_create(-11,6.8);
  vertices[3] = vtx_create(-11,-13.4);
  vertices[4] = vtx_create(-.8,-14.7);
  vertices[5] = vtx_create(-5.4,-9.5);
  vertices[6] = vtx_create(-1,1);
  vertices[7] = vtx_create(9.75,0.9);
  vertices[8] = vtx_create(2.5,-14.58);
  vertices[9] = vtx_create(14.4,-14.5);

  VTX_NUM = 10; */

  /*vertices[0] = vtx_create(15,4);
  vertices[1] = vtx_create(9,10);
  vertices[2] = vtx_create(2,1);
  vertices[3] = vtx_create(8,3);
  vertices[4] = vtx_create(12,2);

  VTX_NUM = 5;*/
  /*
  vertices[0] = vtx_create(2,6);
  vertices[1] = vtx_create(3,1);
  vertices[2] = vtx_create(9,1);
  vertices[3] = vtx_create(11,8);
  vertices[4] = vtx_create(8,3);
  vertices[5] = vtx_create(6,10);
  vertices[6] = vtx_create(4,3);
  VTX_NUM = 7; */

  /*vertices[0] = vtx_create(3,1);
  vertices[1] = vtx_create(4,4);
  vertices[2] = vtx_create(6,3);
  vertices[3] = vtx_create(5,5);  
  vertices[4] = vtx_create(7,4);
  vertices[5] = vtx_create(6,7);
  vertices[6] = vtx_create(4,6);
  vertices[7] = vtx_create(3,8);
  vertices[8] = vtx_create(1,2); 
  VTX_NUM = 9;*/
  
  /*vertices[0] = vtx_create(14,13);
  vertices[1] = vtx_create(10,10);
  vertices[2] = vtx_create(9,17);
  vertices[3] = vtx_create(8,16);
  vertices[4] = vtx_create(7,17);
  vertices[5] = vtx_create(4,15);
  vertices[6] = vtx_create(6,13);
  vertices[7] = vtx_create(5,9);
  vertices[8] = vtx_create(4,12);
  vertices[9] = vtx_create(1,5);
  vertices[10] = vtx_create(3,3);
  vertices[11] = vtx_create(6,4);
  vertices[12] = vtx_create(10,1);
  vertices[13] = vtx_create(8,7);
  vertices[14] = vtx_create(11,5);
  VTX_NUM = 15;*/

  /*vertices[0] = vtx_create(2,1);
  vertices[1] = vtx_create(8,3);
  vertices[2] = vtx_create(10,5);
  vertices[3] = vtx_create(8,6);
  vertices[4] = vtx_create(11,7);
  vertices[5] = vtx_create(7,8);
  vertices[6] = vtx_create(6,10);
  vertices[7] = vtx_create(13,16);
  vertices[8] = vtx_create(9,19);
  vertices[9] = vtx_create(6,17);
  vertices[10] = vtx_create(4,15);
  vertices[11] = vtx_create(5,13);
  vertices[12] = vtx_create(1,4);
  vertices[13] = vtx_create(3,2);
  VTX_NUM = 14;*/
  
  /*vertices[0] = vtx_create(-5,1);
  vertices[1] = vtx_create(5,1);
  vertices[2] = vtx_create(5,6);
  vertices[3] = vtx_create(3,8);
  vertices[4] = vtx_create(-5,6);
  VTX_NUM = 5;*/
