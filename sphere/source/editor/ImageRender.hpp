#ifndef IMAGE_RENDER_HPP
#define IMAGE_RENDER_HPP

#include <afxwin.h>
#include "../common/rgb.hpp"
#include "DIBSection.hpp"

void
StretchedBlit(CPaintDC& dc, CDIBSection* blit_tile,
              const double pixel_width, const double pixel_height,
              const int image_width, const int image_height,
              const RGBA* src_pixels, RECT* ClientRect,
              int redraw_x, int redraw_y, int redraw_width, int redraw_height);

#endif // IMAGE_RENDER_HPP

