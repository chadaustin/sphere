#ifndef __FILEBUFFER_HPP
#define __FILEBUFFER_HPP


#include <stdio.h>
#include "Buffer.hpp"


class CFileBuffer : public CBuffer
{
public:
  CFileBuffer(const char* filename);
  ~CFileBuffer();

  virtual dword GetSize();
  virtual void  Seek(dword location);
  virtual dword Tell();

  // read functions
  virtual void  Read(int num_bytes, byte* bytes);
  virtual byte  ReadBit();
  virtual byte  ReadByte();
  virtual word  ReadWord();
  virtual dword ReadDword();

  // write functions
  virtual void Write(int num_bytes, const byte* bytes);
  virtual void WriteBit(byte bit);
  virtual void WriteByte(byte b);
  virtual void WriteWord(word w);
  virtual void WriteDword(dword dw);

private:
  virtual void FlushReadBits();
  virtual void FlushWriteBits();

private:
  FILE* file;

  int read_cur_byte;
  int read_bits_left;

  int write_cur_byte;
  int write_bits_left;
};


#endif
