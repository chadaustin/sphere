#ifndef __I32_H__
#define __I32_H__


#include "colorspace.h"
#include "packed.h"


PACKED_STRUCT(IMAGE32)
  byte  signature[4];  // Must be ".i32"
  byte  version;       // Must be 1
  dword width;
  dword height;
  byte  compression;   // 0 = none, 1 = huffman
  byte  reserved[242];

  RGBA* data;
END_STRUCT(IMAGE32)


#ifdef __cplusplus
extern "C" {
#endif

extern bool AllocImage32(IMAGE32* image, int width, int height);
extern bool FreeImage32(IMAGE32* image);
extern bool LoadImage32(IMAGE32* image, const char* filename);
extern bool SaveImage32(IMAGE32* image, const char* filename, int compression);

#ifdef __cplusplus
}
#endif

#endif
