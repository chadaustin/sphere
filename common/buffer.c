#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "buffer.h"


// number of bytes to allocate at a time
#define BLOCK_SIZE 256


#define ROUND_UP(size, block) (((size) + (block) - 1) / (block) * (block))


typedef struct
{
  int   size;
  byte* data;
  int   allocated;
  bool  locked;  // a read or write operating is currently going on

} _BUFFER;

typedef struct
{
  _BUFFER* buffer;
  int      location;
  byte     cur_byte;
  int      bits_left;

} _BUFFER_READ;

typedef struct
{
  _BUFFER* buffer;
  int      location;
  byte     cur_byte;
  int      bits_left;

} _BUFFER_WRITE;


///////////////////////////////////////////////////////////////////////////////

BUFFER CreateEmptyBuffer(void)
{
  _BUFFER* _buffer = (_BUFFER*)malloc(sizeof(_BUFFER));
  _buffer->size      = 0;
  _buffer->data      = NULL;
  _buffer->allocated = 0;
  _buffer->locked    = false;
  return _buffer;
}

///////////////////////////////////////////////////////////////////////////////

BUFFER CreateSizedBuffer(int size)
{
  int allocated = ROUND_UP(size, BLOCK_SIZE);
  _BUFFER* _buffer = (_BUFFER*)malloc(sizeof(_BUFFER));
  _buffer->size      = size;
  _buffer->data      = (byte*)malloc(allocated);
  _buffer->allocated = allocated;
  _buffer->locked    = false;
  return (BUFFER)_buffer;
}

///////////////////////////////////////////////////////////////////////////////

BUFFER CreateBuffer(int size, byte* data)
{
  int allocated = ROUND_UP(size, BLOCK_SIZE);
  _BUFFER* _buffer = (_BUFFER*)malloc(sizeof(_BUFFER));
  _buffer->size      = size;
  _buffer->data      = (byte*)malloc(allocated);
  _buffer->allocated = allocated;
  memcpy(_buffer->data, data, size);
  _buffer->locked    = false;
  return (BUFFER)_buffer;
}

///////////////////////////////////////////////////////////////////////////////

void DestroyBuffer(BUFFER buffer)
{
  _BUFFER* _buffer = (_BUFFER*)buffer;
  assert(_buffer->locked == false);
  free(_buffer->data);
  free(_buffer);
}

///////////////////////////////////////////////////////////////////////////////

int GetBufferSize(BUFFER buffer)
{
  _BUFFER* _buffer = (_BUFFER*)buffer;
  assert(_buffer->locked == false);
  return _buffer->size;
}

///////////////////////////////////////////////////////////////////////////////

byte* GetBufferData(BUFFER buffer)
{
  _BUFFER* _buffer =(_BUFFER*)buffer;
  assert(_buffer->locked == false);
  return _buffer->data;
}

///////////////////////////////////////////////////////////////////////////////

BUFFER_READ OpenBufferRead(BUFFER buffer)
{
  _BUFFER_READ* _buffer_read = (_BUFFER_READ*)malloc(sizeof(_BUFFER_READ));

  _buffer_read->buffer = buffer;
  _buffer_read->buffer->locked = true;

  _buffer_read->location  = 0;
  _buffer_read->bits_left = 0;
  _buffer_read->cur_byte  = 0;

  return _buffer_read;
}

///////////////////////////////////////////////////////////////////////////////

byte ReadBufferBit(BUFFER_READ buffer_read)
{
  _BUFFER_READ* _buffer_read = buffer_read;
  byte bit;

  // if there are no bits in the current byte, fill it
  if (_buffer_read->bits_left == 0)
  {
    _buffer_read->cur_byte  = ReadBufferByte(buffer_read);
    _buffer_read->bits_left = 8;
  }

  // grab the bit and update the current byte
  bit = _buffer_read->cur_byte & 1;
  _buffer_read->cur_byte >>= 1;
  _buffer_read->bits_left--;

  return bit;
}

///////////////////////////////////////////////////////////////////////////////

byte ReadBufferByte(BUFFER_READ buffer_read)
{
  _BUFFER_READ* _buffer_read = (_BUFFER_READ*)buffer_read;
  byte b;

  // skip bits until we are on an even byte boundary
  while (_buffer_read->bits_left != 0)
    ReadBufferBit(buffer_read);

  b = _buffer_read->buffer->data[_buffer_read->location];
  _buffer_read->location++;
  return b;
}

///////////////////////////////////////////////////////////////////////////////

word ReadBufferWord(BUFFER_READ buffer_read)
{
  byte a = ReadBufferByte(buffer_read);
  byte b = ReadBufferByte(buffer_read);
  return (b << 8) + a;
}

///////////////////////////////////////////////////////////////////////////////

dword ReadBufferDword(BUFFER_READ buffer_read)
{
  word a = ReadBufferWord(buffer_read);
  word b = ReadBufferWord(buffer_read);
  return (b << 16) + a;
}

///////////////////////////////////////////////////////////////////////////////

void CloseBufferRead(BUFFER_READ buffer_read)
{
  _BUFFER_READ* _buffer_read = (_BUFFER_READ*)buffer_read;
  _buffer_read->buffer->locked = false;
  free(buffer_read);
}

///////////////////////////////////////////////////////////////////////////////

BUFFER_WRITE OpenBufferWrite(BUFFER buffer)
{
  _BUFFER_WRITE* _buffer_write = (_BUFFER_WRITE*)malloc(sizeof(_BUFFER_WRITE));

  _buffer_write->buffer = buffer;
  assert(_buffer_write->buffer->locked == false);
  _buffer_write->buffer->locked = true;

  // always append
  _buffer_write->location  = _buffer_write->buffer->size;
  _buffer_write->bits_left = 0;
  _buffer_write->cur_byte  = 0;

  return _buffer_write;
}

///////////////////////////////////////////////////////////////////////////////

void WriteBufferBit(BUFFER_WRITE buffer_write, byte bit)
{
  _BUFFER_WRITE* _buffer_write = (_BUFFER_WRITE*)buffer_write;

  // if there are no bits left to write, add more
  if (_buffer_write->bits_left == 0)
  {
    _buffer_write->bits_left = 8;
    _buffer_write->cur_byte  = 0;
  }

  // add the bit to the current byte
  _buffer_write->cur_byte >>= 1;
  _buffer_write->cur_byte += bit << 7;
  _buffer_write->bits_left--;

  // if we're done writing bits, write the byte!
  if (_buffer_write->bits_left == 0)
    WriteBufferByte(buffer_write, _buffer_write->cur_byte);
}

///////////////////////////////////////////////////////////////////////////////

void WriteBufferByte(BUFFER_WRITE buffer_write, byte b)
{
  _BUFFER_WRITE* _buffer_write = (_BUFFER_WRITE*)buffer_write;
  
  // write all pending bits
  while (_buffer_write->bits_left != 0)
    WriteBufferBit(buffer_write, 0);

  // if the allocated size is too small, make it bigger!
  if (_buffer_write->buffer->allocated == _buffer_write->buffer->size)
  {
    _buffer_write->buffer->allocated = ROUND_UP(_buffer_write->buffer->size + 1, BLOCK_SIZE);
    _buffer_write->buffer->data = (byte*)realloc(_buffer_write->buffer->data, _buffer_write->buffer->allocated);
  }
  
  _buffer_write->buffer->data[_buffer_write->buffer->size] = b;
  _buffer_write->buffer->size++;
}

///////////////////////////////////////////////////////////////////////////////

void WriteBufferWord(BUFFER_WRITE buffer_write, word w)
{
  WriteBufferByte(buffer_write, (byte)(w & 0xFF));
  WriteBufferByte(buffer_write, (byte)(w >> 8));
}

///////////////////////////////////////////////////////////////////////////////

void WriteBufferDword(BUFFER_WRITE buffer_write, dword dw)
{
  WriteBufferWord(buffer_write, (word)(dw & 0xFFFF));
  WriteBufferWord(buffer_write, (word)(dw >> 16));
}

///////////////////////////////////////////////////////////////////////////////

void CloseBufferWrite(BUFFER_WRITE buffer_write)
{
  _BUFFER_WRITE* _buffer_write = (_BUFFER_WRITE*)buffer_write;

  // write all pending bits
  while (_buffer_write->bits_left != 0)
    WriteBufferBit(buffer_write, 0);

  // unlock the buffer
  _buffer_write->buffer->locked = false;
  free(buffer_write);
}

///////////////////////////////////////////////////////////////////////////////
