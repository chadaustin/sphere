#include "unix_time.h"

dword GetTime () {
  time_t current;

  current = time(NULL);
  if (current == (time_t)(-1))
    return 0;
  return current;
}
