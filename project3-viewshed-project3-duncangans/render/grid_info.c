#include <stdio.h>
#include "grid.h"

// Read a grid and print basic info about it.
int main(int argc, char** argv) {
  FILE* in_file;
  Grid* in_grid;

  // parse and validate command line parameters
  if (argc != 2) {
    fprintf(stderr, "Usage: grid_info <in-file>\n");
    return 1;
  }
  if (!(in_file = fopen(argv[1], "r"))) {
    fprintf(stderr, "Cannot open %s for reading\n", argv[1]);
    return 1;
  }

  // read and print info
  in_grid = grid_read(in_file);
  fprintf(stdout, "ncols: %d\nnrows: %d\nnodata: %d\nmin: %d\nmax %d\n",
    in_grid->ncols, in_grid->nrows, in_grid->nodata_value,
    in_grid->min_value, in_grid->max_value);

  return 0;
}
