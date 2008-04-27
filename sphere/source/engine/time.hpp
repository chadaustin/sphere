#if   defined(SPHERE_SDL)
#include "sdl/sdl_time.hpp"
#if   defined(MAC)
#include "mac/mac_time.hpp"
#elif defined(WIN32)
#include "win32/win32_time.hpp"
#elif defined(unix)
#include "unix/unix_time.h"
#else
#error unsupported platform
#endif
