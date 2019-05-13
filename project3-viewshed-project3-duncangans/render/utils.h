#ifndef __utils_h
#define __utils_h

#include <stdbool.h>

float minf(float a, float b);
float maxf(float a, float b);

float min4f(float a, float b, float c, float d);
float max4f(float a, float b, float c, float d);

int mini(int a, int b);
int maxi(int a, int b);

int minui(int a, int b);
int maxui(int a, int b);

float dist2d(float a_x, float a_y, float b_x, float b_y);
float dist2di(int a_x, int a_y, int b_x, int b_y);

bool within(int a, int b, int delta);

#define swapi(x, y) (x ^= y ^= x ^= y)

#endif
