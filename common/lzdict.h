#ifndef __LZDICT_H
#define __LZDICT_H


#include "types.h"


// types
typedef void* LZDICT;

typedef struct
{
  int   size;
  byte* data;
} LZBUFFER;


#include "begin_c_prototypes.h"

  LZDICT LZCreateDictionary(int max_entries);
  void   LZDestroyDictionary(LZDICT dict);

  int      LZDictGetNumEntries(LZDICT dict);
  void     LZDictAddEntry(LZDICT dict, LZBUFFER* buffer, byte next);
  bool     LZDictContainsEntry(LZDICT dict, LZBUFFER* buffer, byte next);
  int      LZDictGetCode(LZDICT dict, LZBUFFER* buffer);
  LZBUFFER LZDictGetEntry(LZDICT dict, int code);

#include "end_c_prototypes.h"


#endif
