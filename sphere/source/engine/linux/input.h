#ifndef __INPUT_H__
#define __INPUT_H__


#include "types.h"


// key defines
#define KEY_UP         0
#define KEY_RIGHT      1
#define KEY_DOWN       2
#define KEY_LEFT       3
#define KEY_PRIMARY    4
#define KEY_SECONDARY  5
#define KEY_TERTIARY   6
#define KEY_QUATERNARY 7


#ifdef __cplusplus
extern "C" {
#endif


extern bool RefreshInput(void);
extern bool AnyKeyPressed();
extern bool KeyPressed(int key);
extern void WhileKeyPressed(void);
extern void WhileNotKeyPressed(void);


#ifdef __cplusplus
}
#endif


#endif