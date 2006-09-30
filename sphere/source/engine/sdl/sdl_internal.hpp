#ifndef SDL_INTERNAL_HPP
#define SDL_INTERNAL_HPP

#include "../system.hpp"


// video
extern bool InitVideo(int x, int y);
extern bool CloseVideo();
extern void ToggleFPS();

// audio
extern bool InitAudio();
extern bool CloseAudio();

// input
extern bool InitInput();
extern bool CloseInput();
extern void OnKeyDown(int virtual_key);
extern void OnKeyUp(int virtual_key);
extern void OnMouseMove(int x, int y);
extern void OnMouseDown(int button);
extern void OnMouseUp(int button);

// network
extern bool InitNetworkSystem();
extern bool CloseNetworkSystem();

// screenshot
extern bool ShouldTakeScreenshot;
extern void SetScreenshotDirectory(const char* directory);
extern void TakeScreenshot();



#endif
