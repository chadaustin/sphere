#ifndef __HQ2X_H
#define __HQ2X_H

#include "../../common/types.h"
#include "../../common/rgb.hpp"

void hq2x_32(dword* dst, dword* src, int src_width, int src_height);
void hq2x_24(BGR* dst, BGR* src, int src_width, int src_height);

#endif
