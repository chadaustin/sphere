#if defined(SPHERE_SDL)

  #include "sdl/sdl_input.hpp"

#elif defined(_WIN32)

  #include "win32/win32_input.hpp"

#elif defined(unix)

  #include "unix/unix_input.h"

#elif defined(__linux__) || defined(__UNIX__)

  #include "linux/input.hpp"

#else

  #error unsupported platform

#endif
