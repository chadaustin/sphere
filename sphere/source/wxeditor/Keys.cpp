// include the struct definitions
#include "Keys.hpp"

// define the static struct members, which we wouldn't have to do if VC++ didn't suck

#define DEFINE_KEY(name, T, def)                \
    const char* const name::keyname = #name; \
    const T name::default_value = def;

#include "Keys.table"
