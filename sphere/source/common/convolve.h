#ifndef CONVOLVE_HPP
#define CONVOLVE_HPP

void
int_convolve_rgba(int x, int y, int w, int h, int width, int height,
                  RGBA* pixels, int mask_width, int mask_height,
                  int mask_xoffset, int mask_yoffset, const int* mask,
                  int divisor, int offset, int wrap, int clamp, int infinite,
                  int use_red, int use_green, int use_blue, int use_alpha);

#define CONVOLUTION_PIXEL_TYPE RGBA
#define CONVOLUTION_PIXEL_SIZE sizeof(RGBA)
#define CONVOLUTION_PIXEL_CHANNEL_1 old_pixels[py * width + px].red
#define CONVOLUTION_PIXEL_CHANNEL_2 old_pixels[py * width + px].green
#define CONVOLUTION_PIXEL_CHANNEL_3 old_pixels[py * width + px].blue
#define CONVOLUTION_PIXEL_CHANNEL_4 old_pixels[py * width + px].alpha
#define CONVOLUTION_CURRENT_PIXEL_CHANNEL_1 pixels[sy * width + sx].red
#define CONVOLUTION_CURRENT_PIXEL_CHANNEL_2 pixels[sy * width + sx].green
#define CONVOLUTION_CURRENT_PIXEL_CHANNEL_3 pixels[sy * width + sx].blue
#define CONVOLUTION_CURRENT_PIXEL_CHANNEL_4 pixels[sy * width + sx].alpha

#define CONVOLUTION_TYPE int
#define CONVOLUTION_NAME int_convolve_rgba
#include "convolve.c"
#undef CONVOLUTION_TYPE
#undef CONVOLUTION_NAME


void
double_convolve_rgba(int x, int y, int w, int h, int width, int height,
                     RGBA* pixels, int mask_width, int mask_height,
                     int mask_xoffset, int mask_yoffset, const double* mask,
                     int divisor, int offset, int wrap, int clamp, int infinite,
                     int use_red, int use_green, int use_blue, int use_alpha);

#define CONVOLUTION_TYPE double
#define CONVOLUTION_NAME double_convolve_rgba
#include "convolve.c"
#undef CONVOLUTION_TYPE
#undef CONVOLUTION_NAME

#undef CONVOLUTION_PIXEL_TYPE
#undef CONVOLUTION_PIXEL_SIZE
#undef CONVOLUTION_PIXEL_CHANNEL_1
#undef CONVOLUTION_PIXEL_CHANNEL_2
#undef CONVOLUTION_PIXEL_CHANNEL_3
#undef CONVOLUTION_PIXEL_CHANNEL_4
#undef CONVOLUTION_CURRENT_PIXEL_CHANNEL_1
#undef CONVOLUTION_CURRENT_PIXEL_CHANNEL_2
#undef CONVOLUTION_CURRENT_PIXEL_CHANNEL_3
#undef CONVOLUTION_CURRENT_PIXEL_CHANNEL_4

#endif
