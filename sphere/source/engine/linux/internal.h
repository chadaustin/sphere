#ifndef __INTERNAL_H
#define __INTERNAL_H


#include "types.h"


#ifdef __cplusplus
extern "C" {
#endif


extern void UpdateSystem(void);


// graphics.c
extern bool InitGraphics(void);
extern bool CloseGraphics(void);

// sound.c
extern bool InitSoundSystem(void);
extern bool CloseSoundSystem(void);

// input.c
extern byte CurrentKeyBuffer[1024];

extern bool InitInput(void);
extern bool CloseInput(void);


#ifdef __cplusplus
}
#endif


#endif
