/* Terrain visualization */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <float.h>
#include <math.h>
#include "utils.h"
#include "gmath.h"
#include "colorizer.h"
#include "grid.h"
#include "render.h"
#include "vis.h"



#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif





// Constants
int       render_simp_target = 1000;
float     render_nodata_color[3] = {1.0, 1.0, 0.0};
float     render_viewpoint_color[3] = {0.0, 1.0, 1.0};
float     render_visible_color[3] = {1.0, 0.0, 1.0};
int       render_viewpoint_radius = 1;
int       render_num_colorizers = 4;
Colorizer render_colorizers[4];
float     render_color_exponent_delta = 0.05;
float     render_height_proportion = 0.25;
float     render_height_scale_delta = 0.25;
float     render_translation_delta = 0.05;
float     render_rotation_delta = 0.05;
int       render_window_size = 1000;
int       render_window_offset = 100;

// Globals
Grid*     render_elev_grid;
Grid*     render_color_grid;
Grid*     render_vshed_grid;
float     render_color_exponent = 1.0;
bool      render_3d = false;
bool      render_vshed = false;
int       render_v_r;
int       render_v_c;
int       render_simp_factor;
int       render_use_colorizer_idx = 0;
Colorizer render_colorizer;
float     render_height_scale = 1.0;
bool      render_fill = true;
float     render_x_scale;
float     render_x_shift;
float     render_y_scale;
float     render_y_shift;
float     render_z_scale;
float     render_rotation[4] = {1.0, 0.0, 0.0, 0.0};
float     render_translation[3] = {0.0, 0.0, -2.0};

// Define the colorizer constants.
void render_init_colorizers() {
  Colorizer* greyscale = colorizer_init(1);
  greyscale->rgbs[0] = colorizer_color(0.0, 0.0, 0.0); // black
  greyscale->rgbs[1] = colorizer_color(1.0, 1.0, 1.0); // white
  
  Colorizer* bgr = colorizer_init(2);
  bgr->rgbs[0] = colorizer_color(0.0, 0.0, 1.0); // blue
  bgr->rgbs[1] = colorizer_color(0.0, 1.0, 0.0); // green
  bgr->rgbs[2] = colorizer_color(1.0, 0.0, 0.0); // red
  
  Colorizer* topo = colorizer_init(3);
  topo->rgbs[0] = colorizer_color(0.0, 0.4, 0.0); // dark green
  topo->rgbs[1] = colorizer_color(0.6, 0.8, 0.2); // light green
  topo->rgbs[2] = colorizer_color(0.7, 0.6, 0.4); // light brown
  topo->rgbs[3] = colorizer_color(0.5, 0.2, 0.0); // dark brown
  
  Colorizer* flow = colorizer_init(3);
  flow->rgbs[0] = colorizer_color(1.0, 1.0, 1.0); // white
  flow->rgbs[1] = colorizer_color(1.0, 1.0, 0.6); // yellow
  flow->rgbs[2] = colorizer_color(0.0, 0.6, 1.0); // light blue
  flow->rgbs[3] = colorizer_color(0.0, 0.0, 0.6); // dark blue
  
  render_colorizers[0] = *greyscale;
  render_colorizers[1] = *bgr;
  render_colorizers[2] = *topo;
  render_colorizers[3] = *flow;
}

// Set the drawing color based on the [0, 1] scaled value and current colorizer.
void render_set_color(float scaled_color_value) {
  float colors[3];
  colorizer_calc_colors(render_colorizer, scaled_color_value, colors);
  glColor3fv(colors);
}

// Draw a single vertex for a grid by setting the appropriate color and setting
// the vertex itself.
void render_draw_vertex(unsigned int r, unsigned int c) {
  float x, y, elev, scaled_elev, color_val, scaled_color_val, z;

  x = render_x_shift + (render_x_scale * c);
  y = render_y_shift + (render_y_scale * r);

  if (grid_get_nodata(render_elev_grid, r, c)) {
    glColor3fv(render_nodata_color);
    z = 0.0;
  } else {
    if (render_vshed && within(r, render_v_r, render_viewpoint_radius) &&
                        within(c, render_v_c, render_viewpoint_radius)) {
      glColor3fv(render_viewpoint_color);
    } else if (render_vshed && (grid_get(render_vshed_grid, r, c) == vis_grid_visible)) {
      glColor3fv(render_visible_color);
    } else if(grid_get_nodata(render_color_grid, r, c)) {
      color_val = grid_get(render_elev_grid, r, c);
      scaled_color_val = ((color_val - render_elev_grid->min_value)) / (render_color_grid->max_value - render_color_grid->min_value);
      render_set_color(pow(scaled_color_val, render_color_exponent));
    } else {
      color_val = grid_get(render_color_grid, r, c);
      scaled_color_val = ((color_val - render_color_grid->min_value)) / (render_color_grid->max_value - render_color_grid->min_value);
      render_set_color(pow(scaled_color_val, render_color_exponent));
    }
    if (render_3d) {
      elev = grid_get(render_elev_grid, r, c);
      scaled_elev = (elev - render_elev_grid->min_value) / (render_elev_grid->max_value - render_elev_grid->min_value);
      z = render_z_scale * scaled_elev;
    } else {
      z = 0.0;
    }
  }
  glVertex3f(x, y, z);
}

// Recalc drawing constants needed by for each vertex calculation
void render_display_recalc_constants(void) {
  unsigned int max_rc;

  max_rc = maxi(render_elev_grid->ncols, render_elev_grid->nrows);
  render_x_scale = (2.0 / (render_elev_grid->ncols - 1)) *
                    ((float) render_elev_grid->ncols / max_rc);
  render_x_shift = -((float) render_elev_grid->ncols / max_rc);
  render_y_scale = -(2.0 / (render_elev_grid->nrows - 1)) *
                    ((float) render_elev_grid->nrows / max_rc);
  render_y_shift = ((float) render_elev_grid->nrows / max_rc);
  render_z_scale = render_height_scale * render_height_proportion;
  render_colorizer = render_colorizers[render_use_colorizer_idx];
  
  render_viewpoint_radius = max_rc / 500;
}

// Update the display on window resize
void render_display_reshape(int w, int h) {
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60, (float) w / (float) h, 0.01, 20.0);
}

// Render a grid as a series of triangle strips, either in greyscale
// or color scale depending on render_colorize
void render_display(void) {
  unsigned int r, c;
  float  rotation_matrix[16];

  render_display_recalc_constants();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0.0, 0.0, 0.0,
            0.0, 0.0, -1.0,
            0.0, 1.0, 0.0);
  gmath_quat_as_matrix(render_rotation, rotation_matrix);
  glMultMatrixf(rotation_matrix);
  glTranslatef(render_translation[0], render_translation[1], render_translation[2]);

  glPolygonMode(GL_FRONT, render_fill ? GL_FILL : GL_LINE);
  for (r = 0; r < (render_elev_grid->nrows - render_simp_factor); r += render_simp_factor) {
    glBegin(GL_TRIANGLE_STRIP);
    for (c = 0; c < render_elev_grid->ncols; c += render_simp_factor) {
      render_draw_vertex(r, c);
      render_draw_vertex(r+ render_simp_factor, c);
    }
    glEnd();
  }
  glFlush();
}

// Rotate the view around the camera
void render_rotate(bool ccwise, float x, float y, float z) {
  if (render_3d) {
    float vec[3] = {x, y, z};
    float change_quat[4];
    float angle = ccwise ? render_rotation_delta : -render_rotation_delta;
    gmath_make_quat(vec, angle, change_quat);
    gmath_mult_quat(render_rotation, change_quat, render_rotation);
    glutPostRedisplay();
  }
}

// Translate the camera
void render_translate(bool pos_dir, float x, float y, float z) {
  if (render_3d) {
    float amount = pos_dir ? render_translation_delta : -render_translation_delta;
    float amount_vec[3] = {x * amount, y * amount, z * amount};
    float trans_vec[3];
    gmath_mult_quat_vec(render_rotation, amount_vec, trans_vec);
    gmath_add_vec(render_translation, trans_vec, render_translation);
    glutPostRedisplay();
  }
}

// Increase or decrease the height scale.
void render_adjust_height_scale(float dir) {
  if (render_3d) {
    render_height_scale += (dir * render_height_scale_delta);
    glutPostRedisplay();
  }
}

// Cycle through to the next colorizer.
void render_switch_colorizer() {
  render_use_colorizer_idx = ((render_use_colorizer_idx + 1) % render_num_colorizers);
  glutPostRedisplay();
}

// Decrease the exponent used to normalize color values. A low exponent may be
// appropriate for data with high skew, such as one might get for a flow
// flow accumulation data set.
void render_decrease_color_exponent() {
  if (render_color_exponent - render_color_exponent_delta >= 0.0) {
    render_color_exponent -= render_color_exponent_delta;
  }
  glutPostRedisplay();
}

// Increae the color exponent used to normalize colors (see above).
void render_increase_color_exponent() {
  if (render_color_exponent + render_color_exponent_delta <= 1.0) {
    render_color_exponent += render_color_exponent_delta;
  }
  glutPostRedisplay();
}

// Toggle polygon fill / outline.
void render_switch_polygon_fill() {
  render_fill = !render_fill;
  glutPostRedisplay();
}

// Increase the simp factor, i.e. decrease the resolution of the grid
void render_increase_simp_factor(void) {
  render_simp_factor *= 2;
  glutPostRedisplay();
}

// Decrease the simp factor, i.e. increase the resolution of the grid
void render_decrease_simp_factor(void) {
  if (render_simp_factor > 1) {
    render_simp_factor /= 2;
    glutPostRedisplay();
  }
}

// Find a reasonable initial simplification factor before any drawing starts
void render_init_simp_factor() {
  unsigned int max_rc;

  max_rc = maxi(render_elev_grid->nrows, render_elev_grid->ncols);
  render_simp_factor = 1;
  while ((max_rc / render_simp_factor) > render_simp_target) {
    render_simp_factor *= 2;
  }
}

// Callback for keystrokes
void render_keyboard(unsigned char key, int x, int y) {
  switch(key) {
  case 'x':
    render_rotate(true, 1.0, 0.0, 0.0);
    break;
  case 'X':
    render_rotate(false, 1.0, 0.0, 0.0);
    break;
  case 'y':
    render_rotate(true, 0.0, 1.0, 0.0);
    break;
  case 'Y':
    render_rotate(false, 0.0, 1.0, 0.0);
    break;
  case 'z':
    render_rotate(true, 0.0, 0.0, 1.0);
    break;
  case 'Z':
    render_rotate(false, 0.0, 0.0, 1.0);
    break;
  case 'a':
    render_translate(true, 1.0, 0.0, 0.0);
    break;
  case 'A':
    render_translate(false, 1.0, 0.0, 0.0);
    break;
  case 's':
    render_translate(true, 0.0, 1.0, 0.0);
     break;
  case 'S':
    render_translate(false, 0.0, 1.0, 0.0);
    break;
  case 'w':
    render_translate(true, 0.0, 0.0, 1.0);
    break;
  case 'W':
    render_translate(false, 0.0, 0.0, 1.0);
    break;
  case 'h':
    render_adjust_height_scale(-1.0);
    break;
  case 'H':
    render_adjust_height_scale(1.0);
    break;
  case 'c':
    render_switch_colorizer();
    break;
  case 'v':
    render_decrease_color_exponent();
    break;
  case 'V':
    render_increase_color_exponent();
  case 'f':
    render_switch_polygon_fill();
    break;
  case 'r':
    render_increase_simp_factor();
    break;
  case 'R':
    render_decrease_simp_factor();
    break;
  case 'q':
    exit(0);
    break;
  }
}

void render(Grid* elev_grid, Grid* color_grid, bool in_3d) {
  render_elev_grid = elev_grid;
  render_color_grid = color_grid;
  render_3d = in_3d;

  render_init_colorizers();
  render_init_simp_factor();

  int render_dummy_argc = 1;
  char** render_dummy_argv = malloc(2 * sizeof(char*));
  render_dummy_argv[0] = "render";
  render_dummy_argv[1] = NULL;
  
  glutInit(&render_dummy_argc, render_dummy_argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(render_window_size, render_window_size);
  glutInitWindowPosition(render_window_offset, render_window_offset);
  glutCreateWindow("render");

  glClearColor(0, 0, 0, 0);
  glShadeModel(GL_SMOOTH);
  glutDisplayFunc(render_display);
  glutReshapeFunc(render_display_reshape);
  glutKeyboardFunc(render_keyboard);

  glutMainLoop();
}

void render_2d(Grid* grid) {
  render(grid, grid, false);
}

void render_3d_elev(Grid* elev_grid) {
  render(elev_grid, elev_grid, true);
}

void render_3d_elev_and_color(Grid* elev_grid, Grid* color_grid) {
  render(elev_grid, color_grid, true);
}

void render_3d_elev_and_vshed(Grid* elev_grid, Grid* vshed_grid, int v_r, int v_c) {
  render_vshed = true;
  render_vshed_grid = vshed_grid;
  render_v_r = v_r;
  render_v_c = v_c;
  render(elev_grid, elev_grid, true);
}
