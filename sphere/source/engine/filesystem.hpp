#ifdef SPHERE_SDL

  #include "sdl/sdl_filesystem.hpp"

#elif _WIN32

  #include "win32/win32_filesystem.hpp"

#elif defined(unix)

  #include "unix/unix_filesystem.h"

#elif defined(__linux__) || defined(__UNIX__)

  #include "linux/filesystem.hpp"

#else

  #error unsupported platform

#endif
