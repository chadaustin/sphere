// Lots of useful stuff goes here!  =)

#ifndef __XPP_HPP
#define __XPP_HPP


#ifndef __cplusplus
#error Must compile with C++
#endif


#include <string.h>
#include <ctype.h>
#include "unicode.h"


// fix Visual C++'s for scoping problem
#ifdef _MSC_VER
#define for if (false) ; else for
#endif


// add until keyword!
#define until(exp) while(!exp)



#define arraysize(a) (sizeof(a) / sizeof(a[0]))


template<typename T>
T* resize(T*& old_p, unsigned old_size, unsigned new_size)
{  
  if (new_size == 0)
  {
    delete[] old_p;
    old_p = NULL;
  }
  else
  {
    T* new_p = new T[new_size];

    if (old_p != NULL && old_size != 0)
    {
      for (unsigned i = 0; i < (old_size < new_size ? old_size : new_size); i++)
        new_p[i] = old_p[i];
    }

    if (old_p != NULL)
      delete[] old_p;

    old_p = new_p;
  }
  return old_p;
}


template<typename T>
T* duplicate(const T* old, int num_elements)
{
  T* _new = new T[num_elements];
  for (int i = 0; i < num_elements; i++)
    _new[i] = old[i];
  return _new;
}


inline tchar* newstr(const tchar* str)
{
  tchar* ns = new tchar[tstrlen(str) + 1];
  tstrcpy(ns, str);
  return ns;
}



// case insensitive strcmp
inline int strcmp_ci(const tchar* str1, const tchar* str2)
{
  tchar* _str1 = newstr(str1);
  tchar* _str2 = newstr(str2);
  for (tchar* i = _str1; *i; i++)
    *i = ttoupper(*i);
  for (tchar* i = _str2; *i; i++)
    *i = ttoupper(*i);
  return (strcmp(_str1, _str2));
}



template<typename T>
inline void bracket(T& var, const T& min, const T& max)
{
  if (var < min)
    var = min;
  else if (var > max)
    var = max;
}



#endif
