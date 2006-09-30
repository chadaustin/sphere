
#if defined(SPHERE_SDL)

#include "sdl/sdl_audio.hpp"
#elif defined(WIN32)
#include "win32/win32_audio.hpp"
#elif defined(unix)
#include "unix/unix_audio.h"
#else
#error unsupported platform
#endif
