#ifndef __2xSAIWIN_H
#define __2xSAIWIN_H


#include "../../common/types.h"


#ifdef __cplusplus
extern "C" {
#endif
  
  void _2xSaIBitmap(int bit_depth, word* src, int src_width, int src_height, word* depth, int dest_pitch);

#ifdef __cplusplus
}
#endif


#endif
