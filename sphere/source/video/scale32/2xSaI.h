#ifndef __2xSAIWIN_H
#define __2xSAIWIN_H

#include "../../common/types.h"
#include "../../common/rgb.hpp"

void _2xSaI_24(BGR* dst, BGR* src, int src_width, int src_height);
void _2xSaI_32(dword* dst, dword* src, int src_width, int src_height);

#endif
