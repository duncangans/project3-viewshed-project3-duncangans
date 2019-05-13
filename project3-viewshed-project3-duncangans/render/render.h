#include "grid.h"

void render_2d(Grid* grid);
void render_3d_elev(Grid* elev_grid);
void render_3d_elev_and_color(Grid* elev_grid, Grid* color_grid);
void render_3d_elev_and_vshed(Grid* elev_grid, Grid* vshed_grid, int v_r, int v_c);
