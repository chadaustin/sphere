// lz.h
// Chad Austin
// 11.1.1999
// Lempel-Ziv compression/decompression

#ifndef __LZ_H
#define __LZ_H


#include "buffer.h"


#include "begin_c_prototypes.h"

  extern BUFFER LZEncode(BUFFER source);
  extern BUFFER LZDecode(BUFFER source);

#include "end_c_prototypes.h"


#endif
