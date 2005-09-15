#if defined(SPHERE_SDL)

  #include "sdl/sdl_input.hpp"

#elif defined(WIN32)

  #include "win32/win32_input.hpp"

#elif defined(unix)

  #include "unix/unix_input.h"

#else

  #error unsupported platform

#endif

