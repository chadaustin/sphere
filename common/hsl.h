#ifndef __HSL_H
#define __HSL_H



#include "types.h"
#include "packed.h"



PACKED_STRUCT(HSL)
  byte hue;
  byte saturation;
  byte luminance;
END_STRUCT(HSL)

PACKED_STRUCT(AHSL)
  byte alpha;
  byte hue;
  byte saturation;
  byte luminance;
END_STRUCT(AHSL)

PACKED_STRUCT(HSLA)
  byte hue;
  byte saturation;
  byte luminance;
  byte alpha;
END_STRUCT(HSLA)



#endif
