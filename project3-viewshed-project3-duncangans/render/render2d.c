#include <stdio.h>
#include "grid.h"
#include "render.h"

int render2d_max_side = 50000;

// Render a grid in 2 dimensions.
int main(int argc, char** argv) {
  FILE* in_file;
  Grid* grid;

  // parse and validate command line arguments
  if (argc != 2) {
    fprintf(stderr, "Usage: render2d <grid-file>\n");
    return 1;
  }

  if (!(in_file = fopen(argv[1], "r"))) {
    fprintf(stderr, "Cannot open %s for reading\n", argv[1]);
    return 1;
  }

  // read with sampling to ensure a reasonable grid size, then render
  grid = grid_read_simp(in_file, render2d_max_side);
  render_2d(grid);
  return 0;
}
