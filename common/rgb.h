#ifndef __RGB_H
#define __RGB_H



#include "types.h"
#include "packed.h"



PACKED_STRUCT(RGB)
  byte red;
  byte green;
  byte blue;
END_STRUCT(RGB)

PACKED_STRUCT(BGR)
  byte blue;
  byte green;
  byte red;
END_STRUCT(BGR)

PACKED_STRUCT(ARGB)
  byte alpha;
  byte red;
  byte green;
  byte blue;
END_STRUCT(ARGB)

PACKED_STRUCT(ABGR)
  byte alpha;
  byte blue;
  byte green;
  byte red;
END_STRUCT(ABGR)

PACKED_STRUCT(RGBA)
  byte red;
  byte green;
  byte blue;
  byte alpha;
END_STRUCT(RGBA)

PACKED_STRUCT(BGRA)
  byte blue;
  byte green;
  byte red;
  byte alpha;
END_STRUCT(BGRA)



#include "begin_c_prototypes.h"


// ----------------- //
// predefined colors //
// ----------------- //

// RGB

extern RGB rgbBlack;
extern RGB rgbDarkGrey;
extern RGB rgbGrey;
extern RGB rgbLightGrey;
extern RGB rgbWhite;

extern RGB rgbRed;
extern RGB rgbGreen;
extern RGB rgbBlue;

extern RGB rgbCyan;
extern RGB rgbMagenta;
extern RGB rgbYellow;

// BGR

extern BGR bgrBlack;
extern BGR bgrDarkGrey;
extern BGR bgrGrey;
extern BGR bgrLightGrey;
extern BGR bgrWhite;

extern BGR bgrRed;
extern BGR bgrGreen;
extern BGR bgrBlue;

extern BGR bgrCyan;
extern BGR bgrMagenta;
extern BGR bgrYellow;

// ARGB

extern ARGB argbBlack;
extern ARGB argbDarkGrey;
extern ARGB argbGrey;
extern ARGB argbLightGrey;
extern ARGB argbWhite;

extern ARGB argbRed;
extern ARGB argbGreen;
extern ARGB argbBlue;

extern ARGB argbCyan;
extern ARGB argbMagenta;
extern ARGB argbYellow;

// ABGR

extern ABGR abgrBlack;
extern ABGR abgrDarkGrey;
extern ABGR abgrGrey;
extern ABGR abgrLightGrey;
extern ABGR abgrWhite;

extern ABGR abgrRed;
extern ABGR abgrGreen;
extern ABGR abgrBlue;

extern ABGR abgrCyan;
extern ABGR abgrMagenta;
extern ABGR abgrYellow;

// RGBA

extern RGBA rgbaBlack;
extern RGBA rgbaDarkGrey;
extern RGBA rgbaGrey;
extern RGBA rgbaLightGrey;
extern RGBA rgbaWhite;

extern RGBA rgbaRed;
extern RGBA rgbaGreen;
extern RGBA rgbaBlue;

extern RGBA rgbaCyan;
extern RGBA rgbaMagenta;
extern RGBA rgbaYellow;

// BGRA

extern BGRA bgraBlack;
extern BGRA bgraDarkGrey;
extern BGRA bgraGrey;
extern BGRA bgraLightGrey;
extern BGRA bgraWhite;

extern BGRA bgraRed;
extern BGRA bgraGreen;
extern BGRA bgraBlue;

extern BGRA bgraCyan;
extern BGRA bgraMagenta;
extern BGRA bgraYellow;


#include "end_c_prototypes.h"


#endif
