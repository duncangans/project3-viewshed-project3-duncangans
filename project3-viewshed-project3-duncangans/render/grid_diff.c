#include <stdio.h>
#include <stdbool.h>
#include "grid.h"

// Compute and write grid1 - grid2.
int main(int argc, char** argv) {
  FILE* in_file1;
  FILE* in_file2;
  FILE* out_file;
  Grid* in_grid1;
  Grid* in_grid2;
  Grid* out_grid;
  int r, c;

  // parse and validate command line parameters
  if (argc != 4) {
    fprintf(stderr, "Usage: grid_diff <in-file-1> <in-file-2> <out-file>\n");
    return 1;
  }
  if (!(in_file1 = fopen(argv[1], "r"))) {
    fprintf(stderr, "Cannot open %s for reading\n", argv[1]);
    return 1;
  }
  if (!(in_file2 = fopen(argv[2], "r"))) {
    fprintf(stderr, "Cannot open %s for reading\n", argv[2]);
    return 1;
  }
  if (!(out_file = fopen(argv[3], "w"))) {
    fprintf(stderr, "Cannot open %s for writing\n", argv[3]);
  }

  // read grids, calc diff
  in_grid1 = grid_read(in_file1);
  in_grid2 = grid_read(in_file2);
  if ((in_grid1->nrows == in_grid2->nrows) &&
      (in_grid1->ncols == in_grid2->ncols)) {
    out_grid = grid_init_from(in_grid1);
    for (r = 0; r < in_grid1->nrows; r++) {
      for (c = 0; c < in_grid2->ncols; c++) {
        bool in_grid1_nodata = grid_get_nodata(in_grid1, r, c);
        bool in_grid2_nodata = grid_get_nodata(in_grid2, r, c);
        if ((in_grid1_nodata && !in_grid2_nodata) ||
            (!in_grid1_nodata && in_grid2_nodata)) {
          fprintf(stderr, "Nodata mismatch at (%d %d)\n", r, c);
          return 1;
        } else if (in_grid1_nodata && in_grid2_nodata) {
          grid_set_nodata(out_grid, r, c);
        } else {
          grid_set(out_grid, r, c,
            grid_get(in_grid1, r, c) - grid_get(in_grid2, r, c));
        }
      }
    }

    // output
    grid_write(out_file, out_grid);
    return 0;
  } else {
    fprintf(stderr, "Grid sizes do not match\n");
    return 1;
  }
}
