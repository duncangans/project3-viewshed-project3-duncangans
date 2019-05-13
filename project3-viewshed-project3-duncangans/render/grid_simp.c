#include <stdio.h>
#include "grid.h"

// Read a grid and write a downsampled version of it.
// See grid_read_simple.
int main(int argc, char** argv) {
  FILE* in_file;
  FILE* out_file;
  Grid* simp_grid;
  int max_side;

  // parse and validate command line parameters
  if (argc != 4) {
    fprintf(stderr, "Usage: grid_simp <in-file> <max-side> <out-file>\n");
    return 1;
  }
  if (!(in_file = fopen(argv[1], "r"))) {
    fprintf(stderr, "Cannot open %s for reading\n", argv[1]);
    return 1;
  }
  if (!(sscanf(argv[2], "%d", &max_side))) {
    fprintf(stderr, "Cannot parse %s as a max-side value\n", argv[2]);
    return 1;
  }
  if (!(out_file = fopen(argv[3], "w"))) {
    fprintf(stderr, "Cannot open %s for writing\n", argv[3]);
    return 1;
  }

  // compute and write simplification
  simp_grid = grid_read_simp(in_file, max_side);
  grid_write(out_file, simp_grid);

  return 0;
}
