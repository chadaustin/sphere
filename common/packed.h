// Okay, this header works just fine with gcc, but there isn't a way to tell
// Visual C++ to pack a struct.  Therefore, to use this header, every project
// that must have alignment set to 1 byte.

#ifndef __PACKED_H
#define __PACKED_H


#include <assert.h>
#include <stdlib.h>


#if defined(_WIN32)

#define PACKED_STRUCT(name) typedef struct {
#define END_STRUCT(name)    } name;

#elif defined(__linux__) || defined(__UNIX__)

#define PACKED_STRUCT(name) struct __packed_##name {
#define END_STRUCT(name)    } __attribute__((packed)); typedef struct __packed_##name name;

#endif


// checking struct sizes
#define ASSERT_STRUCT_SIZE(name, size)   \
static class __##name##_AssertStructSize \
{                                        \
public:                                  \
  __##name##_AssertStructSize()          \
  {                                      \
    assert(sizeof(name) == size);        \
    if (sizeof(name) != size)            \
      abort();                           \
  }                                      \
} ___##name##_AssertStructSize;


#endif
