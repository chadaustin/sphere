#include "FileBuffer.hpp"


////////////////////////////////////////////////////////////////////////////////

CFileBuffer::CFileBuffer(const char* filename)
: read_cur_byte(0)
, read_bits_left(0)
, write_cur_byte(0)
, write_bits_left(0)
{
  file = fopen(filename, "w+b");
}

////////////////////////////////////////////////////////////////////////////////

CFileBuffer::~CFileBuffer()
{
  FlushReadBits();
  FlushWriteBits();
  fclose(file);
}

////////////////////////////////////////////////////////////////////////////////

dword
CFileBuffer::GetSize()
{
  long cur = ftell(file);
  fseek(file, 0, SEEK_END);
  dword size = ftell(file);
  fseek(file, cur, SEEK_SET);
  return size;
}

////////////////////////////////////////////////////////////////////////////////

void
CFileBuffer::Seek(dword location)
{
  FlushReadBits();
  FlushWriteBits();
  fseek(file, location, SEEK_SET);
}

////////////////////////////////////////////////////////////////////////////////

dword
CFileBuffer::Tell()
{
  FlushReadBits();
  FlushWriteBits();
  return ftell(file);
}

////////////////////////////////////////////////////////////////////////////////

void
CFileBuffer::Read(int num_bytes, byte* bytes)
{
  FlushReadBits();
  FlushWriteBits();
  fread(bytes, 1, num_bytes, file);
}

////////////////////////////////////////////////////////////////////////////////

byte
CFileBuffer::ReadBit()
{
  FlushWriteBits();

  // if there are no bits in the current byte, fill it
  if (read_bits_left == 0)
  {
    read_cur_byte = ReadByte();
    read_bits_left = 8;
  }

  // grab a bit and update the current byte
  byte bit = read_cur_byte & 1;
  read_cur_byte >>= 1;
  read_bits_left--;

  return bit;
}

////////////////////////////////////////////////////////////////////////////////

byte
CFileBuffer::ReadByte()
{
  FlushReadBits();
  FlushWriteBits();
  return fgetc(file);
}

////////////////////////////////////////////////////////////////////////////////

word
CFileBuffer::ReadWord()
{
  byte a = ReadByte();
  byte b = ReadByte();
  return (b << 8) + a;
}

////////////////////////////////////////////////////////////////////////////////

dword
CFileBuffer::ReadDword()
{
  word a = ReadWord();
  word b = ReadWord();
  return (b << 16) + a;
}

////////////////////////////////////////////////////////////////////////////////

void
CFileBuffer::Write(int num_bytes, const byte* bytes)
{
  FlushReadBits();
  FlushWriteBits();
  fwrite(bytes, 1, num_bytes, file);
}

////////////////////////////////////////////////////////////////////////////////

void
CFileBuffer::WriteBit(byte bit)
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

////////////////////////////////////////////////////////////////////////////////

void
CFileBuffer::WriteByte(byte b)
{
  FlushReadBits();
  FlushWriteBits();
  fputc(b, file);
}

////////////////////////////////////////////////////////////////////////////////

void
CFileBuffer::WriteWord(word w)
{
  WriteByte((w >> 0) & 0xFF);
  WriteByte((w >> 8) & 0xFF);
}

////////////////////////////////////////////////////////////////////////////////

void
CFileBuffer::WriteDword(dword dw)
{
  WriteWord((word)((dw >>  0) & 0xFFFF));
  WriteWord((word)((dw >> 16) & 0xFFFF));
}

////////////////////////////////////////////////////////////////////////////////

void
CFileBuffer::FlushReadBits()
{
  read_bits_left = 0;
}

////////////////////////////////////////////////////////////////////////////////

void
CFileBuffer::FlushWriteBits()
{
  while (write_bits_left != 0)
    WriteBit(0);
}

////////////////////////////////////////////////////////////////////////////////
