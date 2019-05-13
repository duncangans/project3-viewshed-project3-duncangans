#ifndef __grid_h
#define __grid_h

#include <stdio.h>
#include <stdbool.h>

typedef struct grid_t {
  int     ncols;
  int     nrows;
  float   xllcorner;
  float   yllcorner;
  float   cellsize;
  float   nodata_value;
  float** data;
  float   min_value;
  float   max_value;
} Grid;

Grid* grid_init_from(Grid* grid);
Grid* grid_init_from_sized(Grid* grid, int nrows, int ncols);
void  grid_free(Grid* grid);
float grid_get(Grid* grid, int r, int c);
void  grid_set(Grid* grid, int r, int c, float val);
bool  grid_get_nodata(Grid* grid, int r, int c);
void  grid_set_nodata(Grid* grid, int r, int c);
Grid* grid_read(FILE* in_file);
Grid* grid_read_simp(FILE* in_file, int max_side);
void  grid_write(FILE* out_file, Grid* grid);

long long int grid_pack_rcpair(Grid* grid, int r, int c);
void          grid_unpack_rcpair(Grid* grid, long long int rcpair, int* r, int* c);

#endif
