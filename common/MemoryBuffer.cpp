#include <stdlib.h>
#include <string.h>
#include "MemoryBuffer.hpp"


// number of bytes to allocate at a time
#define BLOCK_SIZE 4096


#define ROUND_UP(size) (((size) + BLOCK_SIZE - 1) / BLOCK_SIZE * BLOCK_SIZE)


///////////////////////////////////////////////////////////////////////////////

CMemoryBuffer::CMemoryBuffer(dword size, byte* data)
: m_location(0)
, read_cur_byte(0)
, read_bits_left(0)
, write_cur_byte(0)
, write_bits_left(0)
{
  m_size      = size;
  m_allocated = ROUND_UP(size);
  m_data      = (byte*)malloc(m_allocated);  // use malloc and free so we can use realloc
  if (data != NULL)
    memcpy(m_data, data, size);
  else
    memset(m_data, 0, size);
}

///////////////////////////////////////////////////////////////////////////////

CMemoryBuffer::~CMemoryBuffer()
{
  FlushReadBits();
  FlushWriteBits();
  if (m_data != NULL)
    free(m_data);
}

///////////////////////////////////////////////////////////////////////////////

dword
CMemoryBuffer::GetSize()
{
  return m_size;
}

///////////////////////////////////////////////////////////////////////////////

void
CMemoryBuffer::Seek(dword location)
{
  FlushReadBits();
  FlushWriteBits();
  m_location = location;
}

///////////////////////////////////////////////////////////////////////////////

dword
CMemoryBuffer::Tell()
{
  FlushReadBits();
  FlushWriteBits();
  return m_location;
}

///////////////////////////////////////////////////////////////////////////////

void
CMemoryBuffer::Read(int num_bytes, byte* bytes)
{
  FlushReadBits();
  FlushWriteBits();
  memcpy(bytes, m_data + m_location, num_bytes);
}

///////////////////////////////////////////////////////////////////////////////

byte
CMemoryBuffer::ReadBit()
{
  byte bit;

  // if there are no bits in the current byte, fill it
  if (read_bits_left == 0)
  {
    read_cur_byte  = ReadByte();
    read_bits_left = 8;
  }

  // grab the bit and update the current byte
  bit = read_cur_byte & 1;
  read_cur_byte >>= 1;
  read_bits_left--;

  return bit;
}

///////////////////////////////////////////////////////////////////////////////

byte
CMemoryBuffer::ReadByte()
{
  FlushReadBits();
  FlushWriteBits();
  m_location++;
  return m_data[m_location - 1];
}

///////////////////////////////////////////////////////////////////////////////

word
CMemoryBuffer::ReadWord()
{
  byte a = ReadByte();
  byte b = ReadByte();
  return (b << 8) + a;
}

///////////////////////////////////////////////////////////////////////////////

dword
CMemoryBuffer::ReadDword()
{
  word a = ReadWord();
  word b = ReadWord();
  return (b << 16) + a;
}

///////////////////////////////////////////////////////////////////////////////

void
CMemoryBuffer::Write(int num_bytes, const byte* bytes)
{
  for (int i = 0; i < num_bytes; i++)
    WriteByte(bytes[i]);
}

///////////////////////////////////////////////////////////////////////////////

void
CMemoryBuffer::WriteBit(byte bit)
{
  FlushReadBits();

  // if there are no bits left to write, add more
  if (write_bits_left == 0)
  {
    write_bits_left = 8;
    write_cur_byte  = 0;
  }

  // add the bit to the current byte
  write_cur_byte >>= 1;
  write_cur_byte += bit << 7;
  write_bits_left--;

  // if we're done writing bits, write the byte!
  if (write_bits_left == 0)
    WriteByte(write_cur_byte);
}

///////////////////////////////////////////////////////////////////////////////

void
CMemoryBuffer::WriteByte(byte b)
{
  FlushReadBits();
  FlushWriteBits();

  // if the allocated size is too small, make it bigger!
  if (m_allocated == m_size)
  {
    m_allocated = ROUND_UP(m_size + 1);
    m_data = (byte*)realloc(m_data, m_allocated);
  }
  
  m_data[m_size] = b;
  m_size++;
}

///////////////////////////////////////////////////////////////////////////////

void
CMemoryBuffer::WriteWord(word w)
{
  WriteByte((byte)((w >> 0) & 0xFF));
  WriteByte((byte)((w >> 8) & 0xFF));
}

///////////////////////////////////////////////////////////////////////////////

void
CMemoryBuffer::WriteDword(dword dw)
{
  WriteWord((word)((dw >>  0) & 0xFFFF));
  WriteWord((word)((dw >> 16) & 0xFFFF));
}

///////////////////////////////////////////////////////////////////////////////

void
CMemoryBuffer::FlushReadBits()
{
  read_bits_left = 0;
}

///////////////////////////////////////////////////////////////////////////////

void
CMemoryBuffer::FlushWriteBits()
{
  while (write_bits_left != 0)
    WriteBit(0);
}

///////////////////////////////////////////////////////////////////////////////
