#ifndef __MEMORYBUFFER_HPP
#define __MEMORYBUFFER_HPP


#include <stddef.h>
#include "Buffer.hpp"


class CMemoryBuffer : public CBuffer
{
public:
  CMemoryBuffer(dword size = 0, byte* data = NULL);
  ~CMemoryBuffer();

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
  void FlushReadBits();
  void FlushWriteBits();

private:
  dword m_size;
  dword m_allocated;
  byte* m_data;

  dword m_location;

  int read_cur_byte;
  int read_bits_left;

  int write_cur_byte;
  int write_bits_left;
};


#endif
