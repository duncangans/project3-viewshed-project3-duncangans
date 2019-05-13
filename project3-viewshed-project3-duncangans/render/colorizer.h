// Encapsulates one color scheme
typedef struct colorizer_t {
  int     num_buckets;
  float** rgbs;
} Colorizer;

Colorizer* colorizer_init(int num_buckets);
float* colorizer_color(float r, float g, float b);
void colorizer_calc_colors(const Colorizer colorizer, float greyscale, float* colors);
