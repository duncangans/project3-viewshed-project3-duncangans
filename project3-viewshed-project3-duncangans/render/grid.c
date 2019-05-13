/* Utilities for grid files and in-memory grids */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "utils.h"
#include "grid.h"

// Returns an empty grid object.
Grid* grid_init() {
  Grid* grid;
  grid = malloc(sizeof(Grid));
  assert(grid);
  grid->data = NULL;
  grid->min_value = INT_MAX;
  grid->max_value = -INT_MAX;
  return grid;
}

void grid_read_header(FILE* in_file, Grid* grid) {
  float xllcornerf, yllcornerf, cellsizef;
  assert(fscanf(in_file, "ncols %d\n",        &grid->ncols));
  assert(fscanf(in_file, "nrows %d\n",        &grid->nrows));
  assert(fscanf(in_file, "xllcorner %f\n",    &grid->xllcorner));
  assert(fscanf(in_file, "yllcorner %f\n",    &grid->yllcorner));
  assert(fscanf(in_file, "cellsize %f\n",     &grid->cellsize));
  assert(fscanf(in_file, "NODATA_value %f\n", &grid->nodata_value));
}

void grid_write_header(FILE* out_file, Grid* grid) {
  fprintf(out_file, "ncols %d\n",        grid->ncols);
  fprintf(out_file, "nrows %d\n",        grid->nrows);
  fprintf(out_file, "xllcorner %f\n",    grid->xllcorner);
  fprintf(out_file, "yllcorner %f\n",    grid->yllcorner);
  fprintf(out_file, "cellsize %f\n",     grid->cellsize);
  fprintf(out_file, "NODATA_value %f\n", grid->nodata_value);
}

void grid_copy_header(Grid* grid, Grid* new_grid) {
  new_grid->ncols =        grid->ncols;
  new_grid->nrows =        grid->nrows;
  new_grid->xllcorner =    grid->xllcorner;
  new_grid->yllcorner =    grid->yllcorner;
  new_grid->cellsize =     grid->cellsize;
  new_grid->nodata_value = grid->nodata_value;
}

// Ensure that we have allocated space for the grid data.
void grid_malloc_data(Grid* grid) {
  if (!grid->data) {
    grid->data = malloc(grid->nrows * sizeof(float*));
    assert(grid->data);
    int r;
    for (r = 0; r < grid->nrows; r++) {
      grid->data[r] = malloc(grid->ncols * sizeof(float));
      assert(grid->data[r]);
    }
  }
}

// Initialize a grid based on an existing grid, copying e.g. its dimensions.
Grid* grid_init_from(Grid* grid) {
  Grid* new_grid = grid_init();
  grid_copy_header(grid, new_grid);
  grid_malloc_data(new_grid);
  return new_grid;
}

// Initialize an empty grid based on an existing grid, but with new dimensions.
Grid* grid_init_from_sized(Grid* grid, int nrows, int ncols) {
  Grid* new_grid = grid_init();
  grid_copy_header(grid, new_grid);
  new_grid->nrows = nrows;
  new_grid->ncols = ncols;
  grid_malloc_data(new_grid);
  return new_grid;
}

// Free a grid and its associated malloced data;
void grid_free(Grid* grid) {
  int r;
  for (r = 0; r < grid->nrows; r++) {
    free(grid->data[r]);
  }
  free(grid->data);
  free(grid);
}

// Returns the value at the specified point in the grid.
float grid_get(Grid* grid, int r, int c) {
  return grid->data[r][c];
}

// Sets the value at the specified point in the grid.
void grid_set(Grid* grid, int r, int c, float val) {
  grid->data[r][c] = val;
  if (val != grid->nodata_value) {
    grid->min_value = minf(val, grid->min_value);
    grid->max_value = maxf(val, grid->max_value);
  }
}

// Returns true iff the point on the grid has a nodata elev value.
bool grid_get_nodata(Grid* grid, int r, int c) {
  return grid->nodata_value == grid_get(grid, r, c);
}

// Marks the specified point as having nodata.
void grid_set_nodata(Grid* grid, int r, int c) {
  grid_set(grid, r, c, grid->nodata_value);
}

void grid_read_data(FILE* in_file, Grid* grid) {
  int r, c;
  float val;
  for (r = 0; r < grid->nrows; r++) {
    for (c = 0; c < grid->ncols; c++) {
      assert(fscanf(in_file, "%f ", &val));
      grid_set(grid, r, c, val);
    }
  }
}

// Returns a grid read in from a given asc file.
Grid* grid_read(FILE* in_file) {
  Grid* grid = grid_init();
  grid_read_header(in_file, grid);
  grid_malloc_data(grid);
  grid_read_data(in_file, grid);
  return grid;
}

// Returns a sample of the grid read in from the given asc file. If either
// nrows or ncols of the grid data is greater than max_side, downsamples the
// the grid so that nrows and ncols are less than max_side in the returned
// grid. If neither nrows or ncols of the grid data are greater than max_side,
// this function returns the same valuea as grid_read.
Grid* grid_read_simp(FILE* in_file, int max_side) {
  Grid* grid;
  int r, c;
  float val;
  
  grid = grid_init();
  grid_read_header(in_file, grid);
  
  // ratio of actual to max {rows,cols}
  float row_ratio = ((float) grid->nrows) / ((float) max_side);
  float col_ratio = ((float) grid->ncols) / ((float) max_side);
  
  // fail fast to simple case of not needing simplification
  if (row_ratio <= 1.0 && col_ratio <= 1.0) {
    grid_malloc_data(grid);
    grid_read_data(in_file, grid);
    return grid;
  }

  // otherwise we need to simplify
  // round up the largest ratio to get the size of the stride we will need
  // across both rows and colums to achieve the simplification while
  // maintaining perspective. keep the number of raw {rows,cols} for reference
  int raw_nrows = grid->nrows;
  int raw_ncols = grid->ncols;
  int stride = ((int) maxf(row_ratio, col_ratio)) + 1;
  grid->nrows = (grid->nrows / stride) + 1;
  grid->ncols = (grid->ncols / stride) + 1;

  // malloc data to hold only the simplified grid
  grid_malloc_data(grid);

  // do the actual reading and sample grid construction, being mindful of the
  // stride. note that there is no getting around fscanf'ing every elev value in
  // the  raw file
  for (r = 0; r < raw_nrows; r++) {
    for (c = 0; c < raw_ncols; c++) {
      assert(fscanf(in_file, "%f ", &val));
      if ((r % stride == 0) && (c % stride == 0)) {
        grid_set(grid, r/stride, c/stride, val);
      }
    }
  }
  return grid;
}

// Write the complete asc file for a grid.
void grid_write(FILE* out_file, Grid* grid) {
  int r, c;
  grid_write_header(out_file, grid);
  for (r = 0; r < grid->nrows; r++) {
    for (c = 0; c < grid->ncols; c++) {
      fprintf(out_file, "%f ", grid_get(grid, r, c));
    }
    fprintf(out_file, "\n");
  }
}

// Pack a r,c pair into a single int.
long long int grid_pack_rcpair(Grid* grid, int r, int c) {
  return (long long int) (grid->ncols * r) + c;
}

// Unpack a r,c pair from a single int, as packed above.
void grid_unpack_rcpair(Grid* grid, long long int rcpair, int* r, int* c) {
  *c = (int) (rcpair % grid->ncols);
  *r = (int) ((rcpair - *c) / grid->ncols);
}
