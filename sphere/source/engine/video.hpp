#if defined(SPHERE_SDL)

  #include "sdl/sdl_video.hpp"

#elif defined(_WIN32)

  #include "win32/win32_video.hpp"

#elif defined(unix)

  #include "unix/unix_video.h"

#else

  #error unsupported platform

#endif
