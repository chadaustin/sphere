#if defined(SPHERE_SDL)

  #include "sdl/sdl_time.hpp"

#elif defined(_WIN32)

  #include "win32/win32_time.hpp"

#elif defined(__linux__) || defined(__UNIX__)

  #include "linux/timer.hpp"

#else

  #error unsupported platform

#endif
