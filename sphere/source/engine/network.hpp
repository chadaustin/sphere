#ifdef SPHERE_SDL

  #include "sdl/sdl_network.hpp"

#elif _WIN32

  #include "win32/win32_network.hpp"

#elif defined(__linux__) || defined(__UNIX__)

  #include "linux/win32_network.hpp"

#else

  #error unsupported platform

#endif
