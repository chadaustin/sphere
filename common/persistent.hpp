// persistent.hpp
// Chad Austin
// (c) Vermeer 1999
// 6.21.1999
// TPersistent<T> template



#ifndef __PERSISTENT_HPP
#define __PERSISTENT_HPP


#include <string.h>
#include "../common/types.h"


namespace _PERSISTENT
{
  extern bool  Load(const char* key, int* itemnumber, int datalength);
  extern void  Save(int itemnumber);
  extern byte* GetData(int itemnumber);
}


template <typename T>
class TPersistent
{
public:
  TPersistent(const char* key);
  TPersistent(const char* key, const T& orig);
  ~TPersistent();

  operator T() const;
  const T& operator=(T t);
  const T& operator++();
  T        operator++(int);
  const T& operator--();
  T        operator--(int);
  const T& operator*=(T t);
  const T& operator/=(T t);
  const T& operator%=(T t);
  const T& operator+=(T t);
  const T& operator-=(T t);
  const T& operator<<=(T t);
  const T& operator>>=(T t);
  const T& operator&=(T t);
  const T& operator^=(T t);
  const T& operator|=(T t);
  
private:
  byte* GetData() const;

  int ItemNumber;
};


template <typename T>
TPersistent<T>::TPersistent(const char* key)
{
  _PERSISTENT::Load(key, &ItemNumber, sizeof(T));
}

template <typename T>
TPersistent<T>::TPersistent(const char* key, const T& orig)
{
  bool already_exists = _PERSISTENT::Load(key, &ItemNumber, sizeof(T));
  // if this is 1st time, use default value
  if( !already_exists ) {
    *(T*)(GetData()) = orig;  
  }
}

template <typename T>
TPersistent<T>::~TPersistent()
{
}


template <typename T>
TPersistent<T>::operator T() const
{
  return *(T*)(GetData());
}


template <typename T>
const T&
TPersistent<T>::operator=(T t)
{
  (*(T*)(GetData())) = t;
  _PERSISTENT::Save(ItemNumber);
  return *(T*)(GetData());
}


template <typename T>
const T&
TPersistent<T>::operator++()
{
  ++(*(T*)(GetData()));
  _PERSISTENT::Save(ItemNumber);
  return *(T*)(GetData());
}


template <typename T>
T
TPersistent<T>::operator++(int)
{
  T retval = *(T*)(GetData());
  (*(T*)(GetData()))++;
  _PERSISTENT::Save(ItemNumber);
  return retval;
}


template <typename T>
const T&
TPersistent<T>::operator--()
{
  --(*(T*)(GetData()));
  _PERSISTENT::Save(ItemNumber);
  return *(T*)(GetData());
}


template <typename T>
T
TPersistent<T>::operator--(int)
{
  T retval = *(T*)(GetData());
  (*(T*)(GetData()))--;
  _PERSISTENT::Save(ItemNumber);
  return retval;
}


template <typename T>
const T&
TPersistent<T>::operator*=(T t)
{
  *(T*)(GetData()) *= t;
  _PERSISTENT::Save(ItemNumber);
  return *(T*)(GetData());
}


template <typename T>
const T&
TPersistent<T>::operator/=(T t)
{
  *(T*)(GetData()) /= t;
  _PERSISTENT::Save(ItemNumber);
  return *(T*)(GetData());
}


template <typename T>
const T&
TPersistent<T>::operator%=(T t)
{
  *(T*)(GetData()) %= t;
  _PERSISTENT::Save(ItemNumber);
  return *(T*)(GetData());
}


template <typename T>
const T&
TPersistent<T>::operator+=(T t)
{
  *(T*)(GetData()) += t;
  _PERSISTENT::Save(ItemNumber);
  return *(T*)(GetData());
}


template <typename T>
const T&
TPersistent<T>::operator-=(T t)
{
  *(T*)(GetData()) -= t;
  _PERSISTENT::Save(ItemNumber);
  return *(T*)(GetData());
}


template <typename T>
const T&
TPersistent<T>::operator<<=(T t)
{
  *(T*)(GetData()) <<= t;
  _PERSISTENT::Save(ItemNumber);
  return *(T*)(GetData());
}


template <typename T>
const T&
TPersistent<T>::operator>>=(T t)
{
  *(T*)(GetData()) >>= t;
  _PERSISTENT::Save(ItemNumber);
  return *(T*)(GetData());
}


template <typename T>
const T&
TPersistent<T>::operator&=(T t)
{
  *(T*)(GetData()) &= t;
  _PERSISTENT::Save(ItemNumber);
  return *(T*)(GetData());
}


template <typename T>
const T&
TPersistent<T>::operator^=(T t)
{
  *(T*)(GetData()) ^= t;
  _PERSISTENT::Save(ItemNumber);
  return *(T*)(GetData());
}


template <typename T>
const T&
TPersistent<T>::operator|=(T t)
{
  *(T*)(GetData()) |= t;
  _PERSISTENT::Save(ItemNumber);
  return *(T*)(GetData());
}


template <typename T>
byte*
TPersistent<T>::GetData() const
{
  return _PERSISTENT::GetData(ItemNumber);
}


#endif
