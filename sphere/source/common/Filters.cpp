#include <stdlib.h>
#include <string.h>
#include "Filters.hpp"
#include <math.h>


////////////////////////////////////////////////////////////////////////////////

void FlipHorizontally(int width, int height, RGBA* pixels)
{
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width / 2; x++) {
      RGBA p = pixels[x];
      pixels[x] = pixels[width - x - 1];
      pixels[width - x - 1] = p;
    }
    pixels += width;
  }
}

////////////////////////////////////////////////////////////////////////////////

void FlipVertically(int width, int height, RGBA* pixels)
{
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height / 2; y++) {
      RGBA p = pixels[y * width + x];
      pixels[y * width + x] = pixels[(height - y - 1) * width + x];
      pixels[(height - y - 1) * width + x] = p;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void Translate(int width, int height, RGBA* pixels, int dx, int dy)
{
  RGBA* old_pixels = new RGBA[width * height];
  memcpy(old_pixels, pixels, width * height * sizeof(RGBA));

  for (int iy = 0; iy < height; iy++) {
    for (int ix = 0; ix < width; ix++) {
      int sx = ix - dx;
      int sy = iy - dy;

      if (sx < 0) {
        sx += width;
      } else if (sx > width - 1) {
        sx -= width;
      }

      if (sy < 0) {
        sy += height;
      } else if (sy > height - 1) {
        sy -= height;
      }

      pixels[iy * width + ix] = old_pixels[sy * width + sx];
    }
  }

  delete[] old_pixels;
}

////////////////////////////////////////////////////////////////////////////////

void RotateCW(int src_width, int src_height, RGBA* pixels)
{
  RGBA* old_pixels = new RGBA[src_width * src_height];
  memcpy(old_pixels, pixels, src_width * src_height * sizeof(RGBA));

  int dest_width = src_height;
  int dest_height = src_width;

  for (int iy = 0; iy < src_height; iy++) {
    for (int ix = 0; ix < src_width; ix++) {
      int dx = src_width - iy - 1;
      int dy = ix;
      pixels[dy * dest_width + dx] = old_pixels[iy * src_width + ix];
    }
  }

  delete[] old_pixels;
}

////////////////////////////////////////////////////////////////////////////////

void RotateCCW(int src_width, int src_height, RGBA* pixels)
{
  RGBA* old_pixels = new RGBA[src_width * src_height];
  memcpy(old_pixels, pixels, src_width * src_height * sizeof(RGBA));

  int dest_width = src_height;
  int dest_height = src_width;

  for (int iy = 0; iy < src_height; iy++) {
    for (int ix = 0; ix < src_width; ix++) {
      int dx = iy;
      int dy = src_width - ix - 1;
      pixels[dy * dest_width + dx] = old_pixels[iy * src_width + ix];
    }
  }

  delete[] old_pixels;
}

////////////////////////////////////////////////////////////////////////////////

int int_compare(const void* x, const void* y) {
  return (int*) y - (int*) x;
}

void Convolve(int width, int height, RGBA* pixels)
{
  int mask_width = 3;
  int mask_height = 3;
  int mask_xoffset = mask_width  / 2;
  int mask_yoffset = mask_height / 2;

  int divisor = 1;
  int offset = 0;
  int wrap = 0;

  int use_red = 1;
  int use_green = 1;
  int use_blue = 1;
  int use_alpha = 0;

  int mask[] = {-1, -1, -1, -1,  9, -1, -1, -1, -1};
  //{1, 1, 1, 1, 1, 1, 1, 1, 1};
  //{1, 1, 0, 1, 1, 0, 0, 0, 0};
  //{0, -1, 0, -1, 5, -1, 0, -1, 0};
  //{1, -2, 1, -2, 5, -2, 1, -2, 1};
  //{-1, -1, -1, -1, 9, -1, -1, -1, -1};
  //{-1, 0, 0, 0, 0, 0, 0, 0, -1};
  //{-2,  0, 0,  0, 4,  0,  0,  0, -2};
  //{-1, -1, -1, -1,  9, -1, -1, -1, -1};
  //{ 0, -1, 0, -1, 4, -1,  0, -1,  0};
  //{-1,  0, 1, -1, 1,  1, -1,  0,  1};
  //{-1,  0, 0,  0, 2,  0,  0,  0, -1};

  int mask_size = mask_width * mask_height;
  int clamp = 1;

  int red;
  int green;
  int blue;
  int alpha;

  int* red_list = NULL;
  int* green_list = NULL;
  int* blue_list = NULL;
  int* alpha_list = NULL;
  int* list_buffer = NULL;

  RGBA* old_pixels = NULL;

  int sx, sy, mx, my;

#if 0
  int median = 0;

  red_list = (int*) malloc(mask_size * sizeof(int));
  if (red_list == NULL) { return; }

  green_list = (int*) malloc(mask_size * sizeof(int));
  if (red_list == NULL) { return; }

  blue_list = (int*) malloc(mask_size * sizeof(int));
  if (blue_list == NULL) { return; }

  alpha_list = (int*) malloc(mask_size * sizeof(int));
  if (alpha_list == NULL) { return; }

  list_buffer = (int*) malloc(mask_size * sizeof(int));
  if (list_buffer == NULL) { return; }
#endif

  old_pixels = (RGBA*) malloc(width * height * sizeof(RGBA));
  if (old_pixels == NULL)
    return;

  memcpy(old_pixels, pixels, width * height * sizeof(RGBA));

  for (sy = 0; sy < height; sy++) {
    for (sx = 0; sx < width; sx++) {

      int red_total = 0;
      int green_total = 0;
      int blue_total = 0;
      int alpha_total = 0;
      int pixels_in_list = 0;

      for (my = 0; my < mask_height; my += 1) {
        for (mx = 0; mx < mask_width; mx += 1) {

          int px = (sx - mask_xoffset + mx);
          int py = (sy - mask_yoffset + my);

          if (wrap) {
            if (px < 0) px = width + px;
            else if (px >= width) px = px - width;
            if (py < 0) py = height + py;
            else if (py >= height) py = py - height;
          }

          if (px >= 0 && px < width && py >= 0 && py < height) {
#if 0
            red_list[my * mask_width + mx] =  old_pixels[py * width + px].red;
            green_list[my * mask_width + mx] = old_pixels[py * width + px].green;
            blue_list[my * mask_width + mx] = old_pixels[py * width + px].blue;
            alpha_list[my * mask_width + mx] = old_pixels[py * width + px].alpha;
#endif
            red_total   += mask[my * mask_width + mx] * old_pixels[py * width + px].red;
            green_total += mask[my * mask_width + mx] * old_pixels[py * width + px].green;
            blue_total  += mask[my * mask_width + mx] * old_pixels[py * width + px].blue;
            alpha_total += mask[my * mask_width + mx] * old_pixels[py * width + px].alpha;
            pixels_in_list += 1;
          }
          else {
#if 0
            red_list[my * mask_width + mx] = 0;
            green_list[my * mask_width + mx] = 0;
            blue_list[my * mask_width + mx] = 0;
            alpha_list[my * mask_width + mx] = 0;
#endif
          }
        }
      }

      red   = red_total;
      green = green_total;
      blue  = blue_total;
      alpha = alpha_total;

#if 0
      if (median) {
        int* lists[] = {red_list, green_list, blue_list, alpha_list};
        for (int i = 0; i < 4; i++) {
          int list_index = 0;
          for (int my = 0; my < mask_height; my++) {
            for (int mx = 0; mx < mask_width; mx++) {
              if (lists[i][my * mask_width + mx] > -1) {
                list_buffer[list_index] = lists[i][my * mask_width + mx];
                list_index += 1;
              }
            }
          }
 
          qsort(list_buffer, list_index, sizeof(int), int_compare);
          int value = (list_index % 2 == 0) ? list_buffer[list_index / 2] : (list_buffer[list_index / 2] + list_buffer[(list_index / 2) - 1]) / 2;

          switch (i) {
            case (0): red   = value; break;
            case (1): green = value; break;
            case (2): blue  = value; break;
            case (3): alpha = value; break;
          }

        }

      }
#endif

      if (divisor) {
         red /= divisor;
         green /= divisor;
         blue /= divisor;
         alpha /= divisor;
      }

      if (offset) {
        red += offset;
        green += offset;
        blue += offset;
        alpha += offset;
      }

      if (clamp) {
        // clamp to [0,255]
        red   = (red   < 0 ? 0 : (red   > 255 ? 255 : red));
        green = (green < 0 ? 0 : (green > 255 ? 255 : green));
        blue  = (blue  < 0 ? 0 : (blue  > 255 ? 255 : blue));
        alpha = (alpha < 0 ? 0 : (alpha > 255 ? 255 : alpha));
      }


      if (use_red) pixels[sy * width + sx].red = red;
      if (use_green) pixels[sy * width + sx].green =  green;
      if (use_blue) pixels[sy * width + sx].blue =  blue;
      if (use_alpha) pixels[sy * width + sx].alpha = alpha;      
    }
  }


#if 0
  free(red_list);
  free(green_list);
  free(blue_list);
  free(alpha_list);
#endif

  free(old_pixels);
}

////////////////////////////////////////////////////////////////////////////////

inline RGBA BlurPixel(int width, int height, RGBA* pixels, int x, int y)
{
  if (x < 0) {
    x += width;
  } else if (x >= width) {
    x -= width;
  }

  if (y < 0) {
    y += height;
  } else if (y >= height) {
    y -= height;
  }

  return pixels[y * width + x];
}

void Blur(int width, int height, RGBA* pixels, int mask_width, int mask_height)
{
  RGBA* old_pixels = new RGBA[width * height];
  memcpy(old_pixels, pixels, width * height * sizeof(RGBA));

  // the greater width/height is, the more blurry the effect
  int mask_xoffset = mask_width / 2;
  int mask_yoffset = mask_height / 2;
  int mask_size = mask_width * mask_height;

#define Blur_pixel(x, y) BlurPixel(width, height, pixels, ix + x, iy + y)

  for (int ix = 0; ix < width; ix++) {
    for (int iy = 0; iy < height; iy++) {
      int destr = 0;
      int destg = 0;
      int destb = 0;
      int desta = 0;

      for (int my = 0; my < mask_height; my++) {
        for (int mx = 0; mx < mask_width; mx++) {
          destr += Blur_pixel(mx - mask_xoffset, my - mask_yoffset).red;
          destg += Blur_pixel(mx - mask_xoffset, my - mask_yoffset).green;
          destb += Blur_pixel(mx - mask_xoffset, my - mask_yoffset).blue;
          desta += Blur_pixel(mx - mask_xoffset, my - mask_yoffset).alpha;
        }
      }
/*
      destr += Blur_pixel(-1, -1).red;
      destr += Blur_pixel(-1,  0).red;
      destr += Blur_pixel(-1,  1).red;
      destr += Blur_pixel( 0, -1).red;
      destr += Blur_pixel( 0,  0).red;
      destr += Blur_pixel( 0,  1).red;
      destr += Blur_pixel( 1, -1).red;
      destr += Blur_pixel( 1,  0).red;
      destr += Blur_pixel( 1,  1).red;

      destg += Blur_pixel(-1, -1).green;
      destg += Blur_pixel(-1,  0).green;
      destg += Blur_pixel(-1,  1).green;
      destg += Blur_pixel( 0, -1).green;
      destg += Blur_pixel( 0,  0).green;
      destg += Blur_pixel( 0,  1).green;
      destg += Blur_pixel( 1, -1).green;
      destg += Blur_pixel( 1,  0).green;
      destg += Blur_pixel( 1,  1).green;

      destb += Blur_pixel(-1, -1).blue;
      destb += Blur_pixel(-1,  0).blue;
      destb += Blur_pixel(-1,  1).blue;
      destb += Blur_pixel( 0, -1).blue;
      destb += Blur_pixel( 0,  0).blue;
      destb += Blur_pixel( 0,  1).blue;
      destb += Blur_pixel( 1, -1).blue;
      destb += Blur_pixel( 1,  0).blue;
      destb += Blur_pixel( 1,  1).blue;

      desta += Blur_pixel(-1, -1).alpha;
      desta += Blur_pixel(-1,  0).alpha;
      desta += Blur_pixel(-1,  1).alpha;
      desta += Blur_pixel( 0, -1).alpha;
      desta += Blur_pixel( 0,  0).alpha;
      desta += Blur_pixel( 0,  1).alpha;
      desta += Blur_pixel( 1, -1).alpha;
      desta += Blur_pixel( 1,  0).alpha;
      desta += Blur_pixel( 1,  1).alpha;
*/

      pixels[iy * width + ix].red   = destr / mask_size;
      pixels[iy * width + ix].green = destg / mask_size;
      pixels[iy * width + ix].blue  = destb / mask_size;
      pixels[iy * width + ix].alpha = desta / mask_size;
    }
  }

#undef Blur_pixel

  delete[] old_pixels;
}

////////////////////////////////////////////////////////////////////////////////

void Noise(int width, int height, RGBA* pixels)
{
  for (int i = 0; i < width * height; i++) {
    int red   = pixels[i].red   + rand() % 33 - 16;
    int green = pixels[i].green + rand() % 33 - 16;
    int blue  = pixels[i].blue  + rand() % 33 - 16;

    // clamp to [0,255]
    red   = (red   < 0 ? 0 : (red   > 255 ? 255 : red));
    green = (green < 0 ? 0 : (green > 255 ? 255 : green));
    blue  = (blue  < 0 ? 0 : (blue  > 255 ? 255 : blue));

    pixels[i].red   = red;
    pixels[i].green = green;
    pixels[i].blue  = blue;
  }
}

////////////////////////////////////////////////////////////////////////////////

void NegativeImage(int width, int height, bool red, bool green, bool blue, bool alpha, RGBA* pixels) {
  int max = 255;

  for (int i = 0; i < width * height; i++) {
    if (red)
      pixels[i].red   = max - pixels[i].red;
    if (green)
      pixels[i].green = max - pixels[i].green;
    if (blue)
      pixels[i].blue  = max - pixels[i].blue;
    if (alpha)
      pixels[i].alpha = max - pixels[i].alpha;
  }
}

////////////////////////////////////////////////////////////////////////////////

void Solarize(int width, int height, int value, RGBA* pixels) {
  int lut[256];

  for (int j = 0; j < 256; ++j) {
    lut[j] = (j > value) ? (255 - j) : j;
  }

  for (int i = 0; i < width * height; i++) {
    pixels[i].red =   lut[pixels[i].red];
    pixels[i].green = lut[pixels[i].green];
    pixels[i].blue =  lut[pixels[i].blue];
  }
}

////////////////////////////////////////////////////////////////////////////////

void BlendImage(int dest_width, int dest_height, int src_width, int src_height, RGBA* dest_pixels, RGBA* src_pixels)
{
  for (int dx = 0; dx < dest_width; dx++) {
    for (int dy = 0; dy < dest_height; dy++) {
      if (dx > src_width - 1)
        break;

      if (dy > src_height - 1)
        break;
      
      // Blend4(dest_pixels[dy * dest_width + dx], src_pixels[dy * src_width + dx], src_pixels[dy * src_width + dx].alpha);
      Blend4(dest_pixels[dy * dest_width + dx], src_pixels[dy * src_width + dx], src_pixels[dy * src_width + dx].alpha);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void AdjustBrightness(int width, int height, RGBA* pixels, int dred, int dgreen, int dblue, int dalpha)
{
  for (int dx = 0; dx < width; dx++) {
    for (int dy = 0; dy < height; dy++) {

      int red =   pixels[dy * width + dx].red   + dred;
      int green = pixels[dy * width + dx].green + dgreen;
      int blue =  pixels[dy * width + dx].blue  + dblue;
      int alpha = pixels[dy * width + dx].alpha + dalpha;

      // clamp to [0,255]
      red   = (red   < 0 ? 0 : (red   > 255 ? 255 : red));
      green = (green < 0 ? 0 : (green > 255 ? 255 : green));
      blue  = (blue  < 0 ? 0 : (blue  > 255 ? 255 : blue));
      alpha = (alpha < 0 ? 0 : (alpha > 255 ? 255 : alpha));

      pixels[dy * width + dx].red   = red;
      pixels[dy * width + dx].green = green;
      pixels[dy * width + dx].blue  = blue;
      pixels[dy * width + dx].alpha = alpha;

    }
  }
}


////////////////////////////////////////////////////////////////////////////////

void GenerateGammaLookup(unsigned char lookup[256], double gamma) {
  int i;

  for (i = 0; i < 256; ++i) {
    if (gamma) {
      int v = (int) (255.0 * (double) (pow((double) i / (double) 255.0, (double) (1.0/gamma))));
      // clamp to [0, 255]
      v = (v < 0 ? 0 : (v > 255 ? 255 : v));
      lookup[i] = v;
    }
    else {
      lookup[i] = 0;
    }
  }
}

void AdjustGamma(int width, int height, RGBA* pixels, double dred, double dgreen, double dblue, double dalpha)
{
  unsigned char rlookup[256];
  unsigned char glookup[256];
  unsigned char blookup[256];
  unsigned char alookup[256];

  GenerateGammaLookup(rlookup, dred);
  GenerateGammaLookup(glookup, dgreen);
  GenerateGammaLookup(blookup, dblue);
  GenerateGammaLookup(alookup, dalpha);

  for (int dx = 0; dx < width; dx++) {
    for (int dy = 0; dy < height; dy++) {
      pixels[dy * width + dx].red   = rlookup[pixels[dy * width + dx].red];
      pixels[dy * width + dx].green = glookup[pixels[dy * width + dx].green];
      pixels[dy * width + dx].blue  = blookup[pixels[dy * width + dx].blue];
      pixels[dy * width + dx].alpha = alookup[pixels[dy * width + dx].alpha];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

