#ifndef __BUFFER_HPP
#define __BUFFER_HPP


#define LITTLE_ENDIAN


#include "types.h"


class CBuffer
{
public:
  virtual ~CBuffer() { };  // I don't know why this must be defined

  virtual dword GetSize() = 0;
  virtual void  Seek(dword location) = 0;
  virtual dword Tell() = 0;

  // read functions
  virtual void  Read(int num_bytes, byte* bytes) = 0;
  virtual byte  ReadBit() = 0;
  virtual byte  ReadByte() = 0;
  virtual word  ReadWord() = 0;
  virtual dword ReadDword() = 0;

  // write functions
  virtual void Write(int num_bytes, const byte* bytes) = 0;
  virtual void WriteBit(byte bit) = 0;
  virtual void WriteByte(byte b) = 0;
  virtual void WriteWord(word w) = 0;
  virtual void WriteDword(dword dw) = 0;
};


#endif
