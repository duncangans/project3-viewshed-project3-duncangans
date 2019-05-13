#ifndef __vis_h
#define __vis_h

#include <stdbool.h>
#include "grid.h"
#include "llist.h"

typedef struct vis_event_t {
  char  event_type;
  int   t_r;
  int   t_c;
  float alpha;
  float distance;
  float gradient;
} VisEvent;

typedef struct vis_square_t {
  int   r;
  int   c;
  int   size;
  float elev;
} VisSquare;

typedef struct vis_square_event_t {
  char  event_type;
  float alpha;
  float distance;
  float gradient;
  VisSquare* t_square;
} VisSquareEvent;

#define vis_end_event   0
#define vis_query_event 1
#define vis_start_event 2

#define vis_grid_occluded 0
#define vis_grid_visible  1

#define vis_grid_not_rooted 0
#define vis_grid_rooted     1

bool   vis_square_contains(VisSquare* square, int r, int c);
Grid*  vis_compute_vshed(Grid* elev_grid, int v_r, int v_c);
int    vis_count_vshed(Grid* vshed_grid);
Grid*  vis_compute_vcount(Grid* elev_grid);
LList* vis_compute_approx_squares(Grid* elev_grid, int epsilon);
Grid*  vis_compute_avshed(Grid* elev_grid, LList* squares, VisSquare* v_square);
Grid*  vis_compute_avcount(Grid* elev_grid, int epsilon);
Grid*  vis_compute_nnvcount(Grid* vcount_grid, int hood_size);
Grid*  vis_compute_svcount(Grid* elev_grid, int square_size);

#endif
