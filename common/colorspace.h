#ifndef __COLORSPACE_H
#define __COLORSPACE_H


#include "rgb.h"
#include "hsl.h"


#include "begin_c_prototypes.h"

  extern HSL RGBtoHSL(RGB rgb);
  extern RGB HSLtoRGB(HSL hsl);

#include "end_c_prototypes.h"


#endif  // #ifdef __COLORSPACE_H
