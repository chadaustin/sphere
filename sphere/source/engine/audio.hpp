#if defined(SPHERE_SDL)

  #include "sdl/sdl_audio.hpp"

#elif defined(_WIN32)

  #include "win32/win32_audio.hpp"

#elif defined(__linux__) || defined(__UNIX__)

  #include "linux/audio.h"

#else

  #error unsupported platform

#endif
