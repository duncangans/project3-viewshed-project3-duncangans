#include <stdio.h>
#include "grid.h"
#include "render.h"

// Render a grid in 3 dimensions.
// If a secondary grid is given, use the first to determine elevations and the
// second to determine coloration.
// If a row and column are given, mark that spot on the terrain when rendering.
int main(int argc, char** argv) {
  FILE* in_elev_file;
  FILE* in_secondary_file;
  Grid* elev_grid;
  Grid* secondary_grid;

  // first sanity check on params
  if (!((argc == 2) || (argc == 3) || (argc == 5))) {
    fprintf(stderr, "Usage: render3d <grid-file> [<color-grid-file>] [viewpoint-row] [viewpoint-col]\n");
    return 1;
  }

  // try to read the required elev file
  if (!(in_elev_file = fopen(argv[1], "r"))) {
    fprintf(stderr, "Cannot open %s for reading\n", argv[1]);
    return 1;
  }
  elev_grid = grid_read(in_elev_file);

  // 2 args means we just need to render this grid
  if (argc == 2) {
    render_3d_elev(elev_grid);
    return 0;

  // otherwise we need the secondary grid, so try to read it
  } else {
    if (!(in_secondary_file = fopen(argv[2], "r"))) {
      fprintf(stderr, "Cannot open %s for reading\n", argv[2]);
      return 1;
    }
    secondary_grid = grid_read(in_secondary_file);


    // 3 args means we are render to render
    if (argc == 3) {
      render_3d_elev_and_color(elev_grid, secondary_grid);
      return 0;

    // 5 args means we should be given a row and column
    } else {
      int v_r, v_c;
      if (!(sscanf(argv[3], "%d", &v_r))) {
        fprintf(stderr, "Cannot parse %s as a row index\n", argv[3]);
        return 1;
      }
      if (!(sscanf(argv[4], "%d", &v_c))) {
        fprintf(stderr, "Cannot parse %s as a column index\n", argv[4]);
      }
      render_3d_elev_and_vshed(elev_grid, secondary_grid, v_r, v_c);
      return 0;
    }
  }
}
