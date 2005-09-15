#if defined(SPHERE_SDL)

  #include "sdl/sdl_time.hpp"

#elif defined(WIN32)

  #include "win32/win32_time.hpp"

#elif defined(unix)

  #include "unix/unix_time.h"

#elif defined(__linux__) || defined(__UNIX__)

  #include "linux/timer.hpp"

#else

  #error unsupported platform

#endif
