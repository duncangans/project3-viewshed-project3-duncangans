/* Map greyscale values to colors */

#include <stdlib.h>
#include <assert.h>
#include "utils.h"
#include "colorizer.h"

// Constructs a colorizer allowing for the speicied number of buckets, and hence
// requiring num_buckets + 1 colors to be added to its rgbs member.
Colorizer* colorizer_init(int num_buckets) {
	Colorizer* colorizer = malloc(sizeof(Colorizer));
	colorizer->num_buckets = num_buckets;
	colorizer->rgbs = malloc((1 + num_buckets) * sizeof(float*));
	assert(colorizer->rgbs);
	return colorizer;
}

// Consturcts a 3-tuple on the heap representing the indicated color.
float* colorizer_color(float r, float g, float b) {
	float* color = malloc(3 * sizeof(float));
	assert(color);
	color[0] = r;
	color[1] = g;
	color[2] = b;
	return color;
}

// Sets rgb calues into the colors array based on the given greyscale value
// and colorizer.
void colorizer_calc_colors(const Colorizer colorizer, float greyscale, float* colors) {
  int bucket_idx, i;
  float bucket_width, bucket_lower_bound, bucket_upper_bound, greyscale_extra;
  float* lower_rgb;
  float* upper_rgb;

  bucket_idx = mini(greyscale * colorizer.num_buckets, colorizer.num_buckets - 1);
  bucket_width = 1.0 / colorizer.num_buckets;
  bucket_lower_bound = bucket_width * bucket_idx;
  bucket_upper_bound = bucket_lower_bound + bucket_width;
  greyscale_extra = greyscale - bucket_lower_bound;
  
  lower_rgb = colorizer.rgbs[bucket_idx];
  upper_rgb = colorizer.rgbs[bucket_idx + 1];
  for (i = 0; i < 3; i++) {
    colors[i] = (((bucket_width - greyscale_extra) * lower_rgb[i]) +
                 (greyscale_extra * upper_rgb[i]) /
		 bucket_width);
  }
}
