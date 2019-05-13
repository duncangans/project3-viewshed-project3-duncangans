#include "utils.h"
#include <math.h>
#include <stdlib.h>

// Returns the minimum of the two float arguments
float minf(float a, float b) {
  return (a > b) ? b : a;
}

// Returns the maximum of the two float arguments
float maxf(float a, float b) {
  return (a > b) ? a : b;
}

// Returns the minimum of four float arguments
float min4f(float a, float b, float c, float d) {
  return minf(a, minf(b, minf(c, d)));
}

// Returns the maximum of four float arguments
float max4f(float a, float b, float c, float d) {
  return maxf(a, maxf(b, maxf(c, d)));
}

// Returns the minimum of the two int arguments
int mini(int a, int b) {
  return (a > b) ? b : a;
}

// Returns the maximum of the two int arguments
int maxi(int a, int b) {
  return (a > b) ? a : b;
}

// Returns the minimum of the two int arguments
int minui(int a, int b) {
  return (a > b) ? b : a;
}

// Returns the maximum of the two int arguments
int maxui(int a, int b) {
  return (a > b) ? a : b;
}

// Returns the euclidian distance between the two float points
float dist2d(float a_x, float a_y, float b_x, float b_y) {
  return sqrtf(pow(b_x - a_x, 2.0) + pow(b_y - a_y, 2.0));
}

// Returns the euclidian distance between the two int points
float dist2di(int a_x, int a_y, int b_x, int b_y) {
  return sqrtf(pow((float) b_x - a_x, 2.0) + pow((float) b_y - a_y, 2.0));
}

// Returns true iff b is no more than delta units from a.
bool within(int a, int b, int delta) {
  return abs(b - a) <= delta;
}
