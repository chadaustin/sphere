#ifndef __BUFFER_H
#define __BUFFER_H


#include "types.h"


typedef void* BUFFER;
typedef void* BUFFER_READ;
typedef void* BUFFER_WRITE;


#include "begin_c_prototypes.h"

  BUFFER CreateEmptyBuffer(void);
  BUFFER CreateSizedBuffer(int size);
  BUFFER CreateBuffer(int size, byte* data);
  void   DestroyBuffer(BUFFER buffer);

  int   GetBufferSize(BUFFER buffer);
  byte* GetBufferData(BUFFER buffer);

  extern BUFFER_READ OpenBufferRead(BUFFER buffer);
  extern byte        ReadBufferBit(BUFFER_READ read);
  extern byte        ReadBufferByte(BUFFER_READ read);
  extern word        ReadBufferWord(BUFFER_READ read);
  extern dword       ReadBufferDword(BUFFER_READ read);
  extern void        CloseBufferRead(BUFFER_READ read);

  extern BUFFER_WRITE OpenBufferWrite(BUFFER buffer);
  extern void         WriteBufferBit(BUFFER_WRITE write, byte bit);
  extern void         WriteBufferByte(BUFFER_WRITE write, byte b);
  extern void         WriteBufferWord(BUFFER_WRITE write, word w);
  extern void         WriteBufferDword(BUFFER_WRITE write, dword dw);
  extern void         CloseBufferWrite(BUFFER_WRITE write);

#include "end_c_prototypes.h"


#endif

