#ifndef FILTERS_HPP
#define FILTERS_HPP


#include "rgb.hpp"


extern void FlipHorizontally(int width, int height, RGBA* pixels);
extern void FlipVertically(int width, int height, RGBA* pixels);
extern void Translate(int width, int height, RGBA* pixels, int dx, int dy);
extern void RotateCW(int width, RGBA* pixels);
extern void RotateCCW(int width, RGBA* pixels);
extern void Blur(int width, int height, RGBA* pixels);
extern void Noise(int width, int height, RGBA* pixels);
extern void NegativeImage(int width, int height, bool red, bool green, bool blue, bool alpha, RGBA* pixels);
extern void Solarize(int width, int height, int value, RGBA* pixels);
extern void BlendImage(int dest_width, int dest_height, int src_width, int src_height, RGBA* dest_pixels, RGBA* src_pixels);

#endif
