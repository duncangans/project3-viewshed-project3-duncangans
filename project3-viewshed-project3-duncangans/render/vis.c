#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include "vis.h"
#include "rbbst.h"
#include "utils.h"
#include "llist.h"

// Returns the angle in radians swept from point (v_r, v_c) to (t_r, t_c). This
// angle is always between 0 and 2PI.
float vis_swept_alpha(float v_r, float v_c, float t_r, float t_c) {
  float y_delta = v_r - t_r;
  float x_delta = v_c - t_c;
  float a = atan2(y_delta, x_delta);
  return (a >= 0) ? a : (2 * M_PI) + a;
}

// Constructs and returns a new VisEvent.
VisEvent* vis_event_init(char event_type, Grid* elev_grid, int v_r, int v_c, int t_r, int t_c, float alpha) {
  VisEvent* vis_event = malloc(sizeof(VisEvent));
  assert(vis_event);
  vis_event->event_type = event_type;
  vis_event->t_r = t_r;
  vis_event->t_c = t_c;
  vis_event->alpha = alpha;
  vis_event->distance = dist2di(v_r, v_c, t_r, t_c);
  vis_event->gradient = ((float) grid_get(elev_grid, t_r, t_c) - grid_get(elev_grid, v_r, v_c)) / vis_event->distance;
  return vis_event;
}

// A comparator to sort events in increasing sweep angle. We break ties
// in order of increasing distance and then in <end, query, start> order to
// ensure that we never have multiple nodes in the active list that have the
// same distance, as this can cause undetermined behavior on deletion.
int vis_events_in_increasing_alpha(const void* elem_a, const void* elem_b) {
  VisEvent* vis_event_a = *((VisEvent**) elem_a);
  VisEvent* vis_event_b = *((VisEvent**) elem_b);

  if (vis_event_a->alpha < vis_event_b->alpha) {
    return -1;
  } else if (vis_event_a->alpha > vis_event_b->alpha) {
    return 1;
  } else {
    if (vis_event_a->distance < vis_event_b->distance) {
      return -1;
    } else if (vis_event_a->distance > vis_event_b->distance) {
      return 1;
    } else {
      return vis_event_a->event_type - vis_event_b->event_type;
    }
  }
}

// Returns a tree value suitable for insertion into the active list that
// corresponds to the given VisEvent, i.e. having the same distance key and
// gradient.
TreeValue* vis_tree_value_for_event(VisEvent* vis_event) {
  TreeValue* tree_value = malloc(sizeof(TreeValue));
  assert(tree_value);
  tree_value->key = vis_event->distance;
  tree_value->gradient = vis_event->gradient;
  return tree_value;
}

// Returns a dummy tree value with a distance larger than any for any real
// target point.
TreeValue* vis_tree_value_dummy() {
  TreeValue* tree_value = malloc(sizeof(TreeValue));
  assert(tree_value);
  tree_value->key = FLT_MAX;
  tree_value->gradient = 0;
  return tree_value;
}

// Compute the viewshed based on the given elev grid from the viewpoint
// (v_r, v_c), returning the viewshed grid. Returns NULL if the given viewpoint
// is a nodata point.
Grid* vis_compute_vshed(Grid* elev_grid, int v_r, int v_c) {
  // we can not reasonably copmute the viewshed from a nodata viewpoint
  assert(!grid_get_nodata(elev_grid, v_r, v_c));

  // initialize the visiblity storage
  Grid* vshed_grid = grid_init_from(elev_grid);

  // initialize the active list. seed the tree with a dummy node since our
  // tree implementation must always have at least 1 node
  int num_non_viewpoint_cells = (elev_grid->nrows * elev_grid->ncols) - 1;
  int num_vis_tree_values = 1 + v_c + num_non_viewpoint_cells;
  int iVTV = 0;
  TreeValue** vis_tree_values = malloc(num_vis_tree_values * sizeof(TreeValue*));
  TreeValue* vis_tree_value = vis_tree_value_dummy();
  vis_tree_values[iVTV++] = vis_tree_value;
  RBTree* active_list = createTree(*vis_tree_value);

  // initialize and populate the events list with the start, end, and query
  // for each point in the grid
  float v_r_f = (float) v_r;
  float v_c_f = (float) v_c;
  int t_r, t_c;
  int num_vis_events = num_non_viewpoint_cells * 3;
  VisEvent** vis_events = malloc(num_vis_events * sizeof(VisEvent*));
  assert(vis_events);
  int i = 0;
  for (t_r = 0; t_r < elev_grid->nrows; t_r++) {
    for (t_c = 0; t_c < elev_grid->ncols; t_c++) {
      float t_r_f = (float) t_r;
      float t_c_f = (float) t_c;
      float alpha_ll, alpha_lr, alpha_ul, alpha_ur, alpha_min, alpha_ct, alpha_max;

      // don't add events for the viewpoint itself
      if (!((t_r == v_r) && (t_c == v_c))) {
        alpha_ll = vis_swept_alpha(v_r_f, v_c_f, t_r_f - 0.5, t_c_f - 0.5);
        alpha_lr = vis_swept_alpha(v_r_f, v_c_f, t_r_f - 0.5, t_c_f + 0.5);
        alpha_ul = vis_swept_alpha(v_r_f, v_c_f, t_r_f + 0.5, t_c_f - 0.5);
        alpha_ur = vis_swept_alpha(v_r_f, v_c_f, t_r_f + 0.5, t_c_f + 0.5);
        alpha_ct = vis_swept_alpha(v_r_f, v_c_f, t_r_f, t_c_f);
        alpha_min = min4f(alpha_ll, alpha_lr, alpha_ul, alpha_ur);
        alpha_max = max4f(alpha_ll, alpha_lr, alpha_ul, alpha_ur);

        // include the cells on the initial sweep line in the active list
        if ((t_r == v_r) && (t_c < v_c)) {
          vis_events[i] =   vis_event_init(vis_query_event, elev_grid, v_r, v_c, t_r, t_c, alpha_ct);
          vis_events[i+1] = vis_event_init(vis_end_event,   elev_grid, v_r, v_c, t_r, t_c, alpha_min);
          vis_events[i+2] = vis_event_init(vis_start_event, elev_grid, v_r, v_c, t_r, t_c, alpha_max);

          vis_tree_value = vis_tree_value_for_event(vis_events[i+2]);
          insertInto(active_list, *vis_tree_value);
          vis_tree_values[iVTV++] = vis_tree_value;
        } else {
          vis_events[i] =   vis_event_init(vis_start_event, elev_grid, v_r, v_c, t_r, t_c, alpha_min);
          vis_events[i+1] = vis_event_init(vis_query_event, elev_grid, v_r, v_c, t_r, t_c, alpha_ct);
          vis_events[i+2] = vis_event_init(vis_end_event,   elev_grid, v_r, v_c, t_r, t_c, alpha_max);
        }
        i += 3;
      }
    }
  }

  // sort the events list
  qsort(vis_events, num_vis_events, sizeof(VisEvent*), vis_events_in_increasing_alpha);

  // we say that the viewpoint is visible
  grid_set(vshed_grid, v_r, v_c, vis_grid_visible);

  // process the sorted events to compute visibility of the points
  for (i = 0; i < num_vis_events; i++) {
    VisEvent* vis_event = vis_events[i];
    char event_type = vis_event->event_type;
    t_r = vis_event->t_r;
    t_c = vis_event->t_c;

    // start event
    if (event_type == vis_start_event) {
      vis_tree_value = vis_tree_value_for_event(vis_event);
      insertInto(active_list, *vis_tree_value);
      vis_tree_values[iVTV++] = vis_tree_value;

    // end event
    } else if (event_type == vis_end_event) {
      deleteFrom(active_list, vis_event->distance);

    // query event
    } else if (event_type == vis_query_event) {
      // points with nodata elevation have nodata visibility
      if (grid_get_nodata(elev_grid, t_r, t_c)) {
        grid_set_nodata(vshed_grid, t_r, t_c);

      // otherwise find in the active list the highest gradient of
      // the points closer to the viewpoint than the target point
      // the target point is visible iff its gradient is >= this gradient
      // from the active list
      } else {
        float target_gradient = vis_event->gradient;
        float max_gradient = findMaxGradientWithinKey(active_list, vis_event->distance);
        grid_set(vshed_grid, t_r, t_c,
          (target_gradient >= max_gradient) ?
          vis_grid_visible : vis_grid_occluded);
      }
    }
  }

  // free resources used internally
  for (i = 0; i < num_vis_events; i++) { free(vis_events[i]); }
  free(vis_events);

  for (i = 0; i < num_vis_tree_values; i++) { free(vis_tree_values[i]); }
  free(vis_tree_values);

  deleteTree(active_list);
  free(active_list);

  // return the vshed result
  return vshed_grid;
}

// Returns the size of the viewshed indicated by the given vshed_grid.
int vis_count_vshed(Grid* vshed_grid) {
  int r, c;
  int count = 0;
  for (r = 0; r < vshed_grid->nrows; r++) {
    for (c = 0; c < vshed_grid->ncols; c++) {
      if (grid_get(vshed_grid, r, c) == vis_grid_visible) {
        count += 1;
      }
    }
  }
  return count;
}

// Computes and counts the exact viewshed at (r,c) for the given grid.
int vis_compute_vcount_at(Grid* elev_grid, int r, int c) {
  Grid* vshed_grid = vis_compute_vshed(elev_grid, r, c);
  int count = vis_count_vshed(vshed_grid);
  grid_free(vshed_grid);
  return count;
}

// Computes the viewshed count for each point in the map and returns a grid
// representing these counts.
Grid* vis_compute_vcount(Grid* elev_grid) {
  // compute vcounts
  Grid* vcount_grid = grid_init_from(elev_grid);
  int r, c;
  for (r = 0; r < elev_grid->nrows; r++) {

    for (c = 0; c < elev_grid->ncols; c++) {
      if (grid_get_nodata(elev_grid, r, c)) {
        grid_set_nodata(vcount_grid, r, c);
      } else {
        grid_set(vcount_grid, r, c, vis_compute_vcount_at(elev_grid, r, c));
      }
    }
  }

  return vcount_grid;
}

// Simple constructor for VisSquare structs.
VisSquare* vis_square_init(int r, int c, int size) {
  VisSquare* vis_square = malloc(sizeof(VisSquare));
  assert(vis_square);
  vis_square->r = r;
  vis_square->c = c;
  vis_square->size = size;
  return vis_square;
}

// Returns a list of maximally sized squares that collectively cover each cell
// in the grid exactly once.
LList* vis_compute_root_squares(Grid* elev_grid) {
  int r, c, j, k;

  // running list of all root squares
  LList* root_squares = llist_init();

  // the root_grid marks which cells have been covered by a root
  Grid* root_grid = grid_init_from(elev_grid);

  // initially no cells are rooted
  for (r = 0; r < root_grid->nrows; r++) {
    for (c = 0; c < root_grid->ncols; c++) {
      grid_set(root_grid, r, c, vis_grid_not_rooted);
    }
  }

  // consider successively smaller squares in an attempt to cover the grid
  // with the largest possible squares. once a square is found to fit, mark all
  // cells within it as rooted
  int size = 2;
  while ((size*2) < mini(root_grid->nrows, root_grid->ncols)) { size *= 2; }
  for (; size >= 1; size /= 2) {
    for (r = 0; r <= (root_grid->nrows - size); r+= size) {
      for (c = 0; c <= (root_grid->ncols - size); c+= size) {
        if (grid_get(root_grid, r, c) == vis_grid_not_rooted) {
          for (j = 0; j < size; j++) {
            for (k = 0; k < size; k++) {
              grid_set(root_grid, r+j, c+k, vis_grid_rooted);
            }
          }
          llist_insert(root_squares, (void*) vis_square_init(r, c, size));
        }
      }
    }
  }

  return root_squares;
}

// A square of all nodata is tight.
// A square of mixed nodata and data is not tight.
// A square of all data is tight iff all of the interior elev values are within
// epsilon of each other.
// The above imply that a square of size 1 is neccisarily tight.
bool vis_square_is_tight(Grid* elev_grid, VisSquare* vis_square, int epsilon) {
  int j, k;
  bool seen_nodata = grid_get_nodata(elev_grid, vis_square->r, vis_square->c);
  int  max_height = grid_get(elev_grid, vis_square->r, vis_square->c);
  int  min_height = max_height;

  for (j = 0; j < vis_square->size; j++) {
    for (k = 0; k < vis_square->size; k++) {
      // mixed nodata and data;
      if (seen_nodata == grid_get_nodata(elev_grid, vis_square->r+j, vis_square->c+k)) {
        max_height = maxi(max_height, grid_get(elev_grid, vis_square->r+j, vis_square->c+k));
        min_height = mini(min_height, grid_get(elev_grid, vis_square->r+j, vis_square->c+k));
      } else {
        return false;
      }
    }
  }
  // all nodata
  if (seen_nodata) {
    return true;
  // all height
  } else {
    return (max_height - min_height) <= epsilon;
  }
}

// Returns true iff the square represents nodata cells.
bool vis_square_is_nodata(VisSquare* vis_square, Grid* elev_grid) {
  return grid_get_nodata(elev_grid, vis_square->r, vis_square->c);
}

// Compute the average elev for the cells represented by the square and assign
// this elev to the square.  Sets the elev to the nodata for the grid if the
// square represents nodata cells.
void vis_square_compute_elev(VisSquare* vis_square, Grid* elev_grid) {
  if (vis_square_is_nodata(vis_square, elev_grid)) {
    vis_square->elev = elev_grid->nodata_value;
  } else {
    long long total = 0;
    int j, k;
    for (j = 0; j < vis_square->size; j++) {
      for (k = 0; k < vis_square->size; k++) {
        total += grid_get(elev_grid, vis_square->r + j, vis_square->c + k);
      }
    }
    vis_square->elev = (int) (total / (vis_square->size * vis_square->size));
  }
}

// Concats onto squares the approx squares given by decomposing root_square
// until all of its consituent squares are tight.
void vis_decompose_square(Grid* elev_grid, VisSquare* root_square, LList* squares, int epsilon) {
  if (vis_square_is_tight(elev_grid, root_square, epsilon)) {
    vis_square_compute_elev(root_square, elev_grid);
    llist_insert(squares, (void*) root_square);
  } else {
    int new_size = root_square->size /2;
    int middle_r = root_square->r + new_size;
    int middle_c = root_square->c + new_size;
    VisSquare* a_square = vis_square_init(root_square->r, root_square->c, new_size);
    VisSquare* b_square = vis_square_init(root_square->r, middle_c,       new_size);
    VisSquare* c_square = vis_square_init(middle_r,       root_square->c, new_size);
    VisSquare* d_square = vis_square_init(middle_r,       middle_c,       new_size);
    vis_decompose_square(elev_grid, a_square, squares, epsilon);
    vis_decompose_square(elev_grid, b_square, squares, epsilon);
    vis_decompose_square(elev_grid, c_square, squares, epsilon);
    vis_decompose_square(elev_grid, d_square, squares, epsilon);
  }
}

// Find all root squares. For each such square, decompose it until all of
// its constituent squares are tight. Return the resulting list, which
// contains the squares to use in approximating the grid.
LList* vis_compute_approx_squares(Grid* elev_grid, int epsilon) {
  LList* root_squares = vis_compute_root_squares(elev_grid);
  LList* approx_squares = llist_init();

  LListNode* root_square_node = llist_head(root_squares);
  while (root_square_node) {
    VisSquare* root_square = (VisSquare*) llist_node_value(root_square_node);
    vis_decompose_square(elev_grid, root_square, approx_squares, epsilon);
    root_square_node = llist_node_next(root_square_node);
  }

  return approx_squares;
}

// Returns true iff the square contains the cell at (r,c).
bool vis_square_contains(VisSquare* square, int r, int c) {
  return ((r >= square->r) && (r < (square->r + square->size)) &&
          (c >= square->c) && (c < (square->c + square->size)));
}

// Returns true iff the square contains a cell that is on the inital sweep
// line for a viewpoint at (v_r, v_c).
bool vis_square_intersects_initial_sweep(VisSquare* square, float v_r, float v_c) {
  return ((v_r >= (square->r - 0.5)) && (v_r < (square->r + square->size - 0.5)) &&
          (square->c < v_c));
}

// Returns the center row value for the square. Will be of the form xx.5
// unless the square has size 1.
float vis_square_center_r(VisSquare* square) {
  return square->r + ((square->size - 1) / 2.0);
}

// Returns the center column value for the square. Will be of the form xx.5
// unless the square has size 1.
float vis_square_center_c(VisSquare*square) {
  return square->c + ((square->size - 1) / 2.0);
}

// Returns a tree value suitable for insertion into the active list that
// corresponds to the given VisSquareEvent, i.e. having the same distance key
// and  gradient.
TreeValue* vis_tree_value_for_square_event(VisSquareEvent* vis_square_event) {
  TreeValue* tree_value = malloc(sizeof(TreeValue));
  assert(tree_value);
  tree_value->key = vis_square_event->distance;
  tree_value->gradient = vis_square_event->gradient;
  return tree_value;
}

// Constructs and returns a new VisSquareEvent.
VisSquareEvent* vis_square_event_init(char event_type, VisSquare* v_square, VisSquare* t_square, float v_r, float v_c, float t_r, float t_c, float alpha) {
  VisSquareEvent* vis_square_event = malloc(sizeof(VisSquareEvent));
  assert(vis_square_event);
  vis_square_event->event_type = event_type;
  vis_square_event->t_square = t_square;
  vis_square_event->alpha = alpha;
  vis_square_event->distance = dist2d(v_r, v_c, t_r, t_c);
  vis_square_event->gradient = (t_square->elev - v_square->elev) / vis_square_event->distance;
  return vis_square_event;
}

// A comparator to sort square events in increasing sweep angle. We break ties
// in order of increasing distance and then in <end, query, start> order to
// ensure that we never have multiple nodes in the active list that have the
// same distance, as this can cause undetermined behavior on deletion.
int vis_square_events_in_increasing_alpha(const void* elem_a, const void* elem_b) {
  VisSquareEvent* vis_square_event_a = *((VisSquareEvent**) elem_a);
  VisSquareEvent* vis_square_event_b = *((VisSquareEvent**) elem_b);

  if (vis_square_event_a->alpha < vis_square_event_b->alpha) {
    return -1;
  } else if (vis_square_event_a->alpha > vis_square_event_b->alpha) {
    return 1;
  } else {
    if (vis_square_event_a->distance < vis_square_event_b->distance) {
      return -1;
    } else if (vis_square_event_a->distance > vis_square_event_b->distance) {
      return 1;
    } else {
      return vis_square_event_a->event_type - vis_square_event_b->event_type;
    }
  }
}

// Compute the aprox. viewshed based on the given elev grid from the viewpoint
// represented by v_square, returning the viewshed grid.
// Returns NULL if the given viewpoint is a nodata sqaure.
Grid* vis_compute_avshed(Grid* elev_grid, LList* squares, VisSquare* v_square) {
  // we can not reasonably compute the viewshed from a nodata viewpoint.
  assert(!vis_square_is_nodata(v_square, elev_grid));

  // initialize the visiblity storage
  Grid* vshed_grid = grid_init_from(elev_grid);

  // initialize the active list. seed the tree with a dummy node since our
  // tree implementation must always have at least 1 node
  LList* vis_tree_values = llist_init();
  TreeValue* vis_tree_value = vis_tree_value_dummy();
  llist_insert(vis_tree_values, vis_tree_value);
  RBTree* active_list = createTree(*vis_tree_value);

  // approximate the viewpoint at the center of the v_square
  float v_r_f = vis_square_center_r(v_square);
  float v_c_f = vis_square_center_c(v_square);

  // initialize and populate the events list with the start, end, and query
  // for each point in the grid
  int num_vis_square_events = (llist_count(squares) - 1) * 3;
  VisSquareEvent** vis_square_events = malloc(num_vis_square_events * sizeof(VisSquareEvent*));
  assert(vis_square_events);
  int i = 0;
  LListNode* square_node = llist_head(squares);
  while (square_node) {
    VisSquare* square = (VisSquare*) llist_node_value(square_node);
    // don't add events for the viewpoint itself
    if (square != v_square) {
      float half_size = ((float) square->size) / 2.0;
      float t_r_f = vis_square_center_r(square);
      float t_c_f = vis_square_center_c(square);
      float alpha_ll = vis_swept_alpha(v_r_f, v_c_f, t_r_f - half_size, t_c_f - half_size);
      float alpha_lr = vis_swept_alpha(v_r_f, v_c_f, t_r_f - half_size, t_c_f + half_size);
      float alpha_ul = vis_swept_alpha(v_r_f, v_c_f, t_r_f + half_size, t_c_f - half_size);
      float alpha_ur = vis_swept_alpha(v_r_f, v_c_f, t_r_f + half_size, t_c_f + half_size);
      float alpha_ct = vis_swept_alpha(v_r_f, v_c_f, t_r_f, t_c_f);
      float alpha_min = min4f(alpha_ll, alpha_lr, alpha_ul, alpha_ur);
      float alpha_max = max4f(alpha_ll, alpha_lr, alpha_ul, alpha_ur);

      // include the cells on the initial sweep line in the active list
      if (vis_square_intersects_initial_sweep(square, v_r_f, v_c_f)) {
        vis_square_events[i] =   vis_square_event_init(vis_query_event, v_square, square, v_r_f, v_c_f, t_r_f, t_c_f, alpha_ct);
        vis_square_events[i+1] = vis_square_event_init(vis_end_event,   v_square, square, v_r_f, v_c_f, t_r_f, t_c_f, alpha_min);
        vis_square_events[i+2] = vis_square_event_init(vis_start_event, v_square, square, v_r_f, v_c_f, t_r_f, t_c_f, alpha_max);

        vis_tree_value = vis_tree_value_for_square_event(vis_square_events[i+2]);
        insertInto(active_list, *vis_tree_value);
        llist_insert(vis_tree_values, vis_tree_value);
      } else {
        vis_square_events[i] =   vis_square_event_init(vis_start_event, v_square, square, v_r_f, v_c_f, t_r_f, t_c_f, alpha_min);
        vis_square_events[i+1] = vis_square_event_init(vis_query_event, v_square, square, v_r_f, v_c_f, t_r_f, t_c_f, alpha_ct);
        vis_square_events[i+2] = vis_square_event_init(vis_end_event,   v_square, square, v_r_f, v_c_f, t_r_f, t_c_f, alpha_max);
      }
      i += 3;
    }
    square_node = llist_node_next(square_node);
  }

  // sort the events list
  qsort(vis_square_events, num_vis_square_events, sizeof(VisSquareEvent*), vis_square_events_in_increasing_alpha);

  // we say that the all cells in the viewpoint's square are visible
  int j, k;
  for (j = 0; j < v_square->size; j++) {
    for (k = 0; k < v_square->size; k++) {
      grid_set(vshed_grid, v_square->r + j, v_square->c + k, vis_grid_visible);
    }
  }

  // process the sorted events to compute visibility of the other sqaures
  for (i = 0; i < num_vis_square_events; i++) {
    VisSquareEvent* vis_square_event = vis_square_events[i];
    char event_type = vis_square_event->event_type;

    // start event
    if (event_type == vis_start_event) {
      vis_tree_value = vis_tree_value_for_square_event(vis_square_event);
      insertInto(active_list, *vis_tree_value);
      llist_insert(vis_tree_values, vis_tree_value);

    // end event
    } else if (event_type == vis_end_event) {
      deleteFrom(active_list, vis_square_event->distance);

    // query event
    } else if (event_type == vis_query_event) {
      // we need the square associated with this event so that we can set
      // visibility for all of the associated cells.
      VisSquare* t_square = vis_square_event->t_square;

      // squares with nodata elevation have nodata visibility
      if (vis_square_is_nodata(t_square, elev_grid)) {
        for (j = 0; j < t_square->size; j++) {
          for (k = 0; k < t_square->size; k++) {
            grid_set_nodata(vshed_grid, t_square->r + j, t_square->c + k);
          }
        }

      // otherwise find in the active list the highest gradient of
      // the squares closer to the viewpoint than the target square
      // the target square is visible iff its gradient is >= this gradient
      // from the active list
      } else {
        float target_gradient = vis_square_event->gradient;
        float max_gradient = findMaxGradientWithinKey(active_list, vis_square_event->distance);
        int visibility = (target_gradient >= max_gradient) ?
                           vis_grid_visible : vis_grid_occluded;
        for (j = 0; j < t_square->size; j++) {
          for (k = 0; k < t_square->size; k++) {
            grid_set(vshed_grid, t_square->r + j, t_square->c + k, visibility);
          }
        }
      }
    }
  }

  // free resources used internally
  for (i = 0; i < num_vis_square_events; i++) { free(vis_square_events[i]); }
  free(vis_square_events);

  llist_destroy_with_values(vis_tree_values);

  deleteTree(active_list);
  free(active_list);

  // return the vshed result
  return vshed_grid;
}

// Computes the approximate viewshed count for every point in the map and
// returns a grid representing these counts.
Grid* vis_compute_avcount(Grid* elev_grid, int epsilon) {
  // simplify the grid into larger squares
  LList* approx_squares = vis_compute_approx_squares(elev_grid, epsilon);

  // compute and count the viewshed for each simplified square
  int j, k, avcount;
  Grid* avcount_grid = grid_init_from(elev_grid);
  LListNode* square_node = llist_head(approx_squares);

  // compute the approximate viewshed for each square and use it as an
  // approximation for all cells in that square
  while(square_node) {
    VisSquare* v_square = (VisSquare*) llist_node_value(square_node);
    if (vis_square_is_nodata(v_square, elev_grid)) {
      avcount = elev_grid->nodata_value;
    } else {
      Grid* avshed_grid = vis_compute_avshed(elev_grid, approx_squares, v_square);
      avcount = vis_count_vshed(avshed_grid);
      grid_free(avshed_grid);
    }

    for (j = 0; j < v_square->size; j++) {
      for (k = 0; k < v_square->size; k++) {
        grid_set(avcount_grid, v_square->r + j, v_square->c + k, avcount);
      }
    }

    square_node = llist_node_next(square_node);
  }

  return avcount_grid;
}

// Returns a viewcount grid corresponding to what would be produced by a
// nearest-neighbor approximation on the elev grid that yielded the given
// vcount grid.
Grid* vis_compute_nnvcount(Grid* vcount_grid, int hood_size) {
  Grid* nnvcount_grid = grid_init_from(vcount_grid);

  int m, n, j, k;
  for (m = 0; m < vcount_grid->nrows; m += hood_size) {
    for (n = 0; n < vcount_grid->ncols; n += hood_size) {
      // find the most central data cell in the neighborhood, and its
      // vcount value. we will use it to approximate the other data cells
      float dist_best = FLT_MAX;
      int count_best = 0;
      float r_center = m + ((hood_size - 1) / 2.0);
      float c_center = n + ((hood_size - 1) / 2.0);
      for (j = 0; (j < hood_size) && (m+j < vcount_grid->nrows); j++) {
        for (k = 0; (k < hood_size) && (n+k < vcount_grid->ncols); k++) {
          if (!grid_get_nodata(vcount_grid, m+j, n+k)) {
            float dist = dist2d(r_center, c_center, (float) m+j, (float) n+k);
            if (dist < dist_best) {
              dist_best = dist;
              count_best = grid_get(vcount_grid, m+j, n+k);
            }
          }
        }
      }

      // for each data cell in the neighborhood, assign it the approximation
      // found above. nodata cells remain nodata
      for (j = 0; (j < hood_size) && (m+j < vcount_grid->nrows); j++) {
        for (k = 0; (k < hood_size) && (n+k < vcount_grid->ncols); k++) {
          if (grid_get_nodata(vcount_grid, m+j, n+k)) {
            grid_set_nodata(nnvcount_grid, m+j, n+k);
          } else {
            grid_set(nnvcount_grid, m+j, n+k, count_best);
          }
        }
      }
    }
  }

  return nnvcount_grid;
}

// Compute the viewshed counts exactly for a every cell in a lower-resolution
// version of the given grid, then use those counts to approximate view counts
// for all cells in the original grid.
Grid* vis_compute_svcount(Grid* elev_grid, int square_size) {
  // determine the size of the simplified grid
  int nsimprows = (elev_grid->nrows / square_size) + 1;
  int nsimpcols = (elev_grid->ncols / square_size) + 1;
  int m, n, j, k;
  Grid* simp_grid = grid_init_from_sized(elev_grid, nsimprows, nsimpcols);

  // for each cell in this simple grid, determine its height based on the
  // corresonding cells int the original elev_grid. if all such original cells
  // are nodata then the simp cell is nodata. if some are have data then the
  // simp cell is the average elev of those cells
  for (m = 0; m < nsimprows; m++) {
    for (n = 0; n < nsimpcols; n++) {
      int data_cells = 0;
      long long total_elev = 0;
      for (j = 0; (j < square_size) && ((m*square_size)+j < elev_grid->nrows); j++) {
        for (k = 0; (k < square_size) && ((n*square_size)+k < elev_grid->ncols); k++) {
          if (!grid_get_nodata(elev_grid, (m*square_size)+j, (n*square_size)+k)) {
            data_cells++;
            total_elev += grid_get(elev_grid, (m*square_size)+j, (n*square_size)+k);
          }
        }
      }
      if (data_cells == 0) {
        grid_set_nodata(simp_grid, m, n);
      } else {
        grid_set(simp_grid, m, n, (int) (total_elev / data_cells));
      }
    }
  }

  // compute the exact viewcount on this simplified grid
  Grid* simp_vcount_grid = vis_compute_vcount(simp_grid);

  // use these viewcounts to fill in viewcount values for corresponding cells
  // in the svcount grid.
  Grid* svcount_grid = grid_init_from(elev_grid);
  for (m = 0; m < nsimprows; m++) {
    for (n = 0; n < nsimpcols; n++) {
      for (j = 0; (j < square_size) && ((m*square_size)+j < elev_grid->nrows); j++) {
        for (k = 0; (k < square_size) && ((n*square_size)+k < elev_grid->ncols); k++) {
          if (grid_get_nodata(elev_grid, (m*square_size)+j, (n*square_size)+k)) {
            grid_set_nodata(svcount_grid, (m*square_size)+j, (n*square_size)+k);
          } else {
            grid_set(svcount_grid, (m*square_size)+j, (n*square_size)+k,
              grid_get(simp_vcount_grid, m, n) * square_size * square_size);
          }
        }
      }
    }
  }

  return svcount_grid;
}
