#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lz.h"
#include "lzdict.h"


#define BITS        12  // 1024 dictionary entries
#define MAX_ENTRIES (1 << BITS)


static LZBUFFER empty = { 0, NULL };


////////////////////////////////////////////////////////////////////////////////

static LZDICT LZCreateDefaultDictionary()
{
  LZDICT dict = LZCreateDictionary(MAX_ENTRIES);
  int i;
  for (i = 0; i < 256; i++)
    LZDictAddEntry(dict, &empty, (byte)i);
  return dict;
}

////////////////////////////////////////////////////////////////////////////////

static void LZWriteCode(LZDICT dict, BUFFER_WRITE buffer_write, int code)
{
  int i;
  assert(code >= 0 && code < LZDictGetNumEntries(dict));
  for (i = BITS - 1; i >= 0; i--)
  {
    int bit = (code & (1 << i)) != 0;
    WriteBufferBit(buffer_write, (byte)bit);
  }
}

////////////////////////////////////////////////////////////////////////////////

BUFFER LZEncode(BUFFER source)
{
  BUFFER result = CreateEmptyBuffer();
  BUFFER_WRITE result_write = OpenBufferWrite(result);
  LZDICT dict = LZCreateDefaultDictionary();
  LZBUFFER w = empty;

  int i;
  int code;

  // while characters are still in input
  for (i = 0; i < GetBufferSize(source); i++)
  {
    // read character K
    byte K = GetBufferData(source)[i];
    
    // if wK is in dictionary
    if (LZDictContainsEntry(dict, &w, K))
    {
      // w = wK
      w.data = (byte*)realloc(w.data, w.size + 1);
      w.data[w.size] = K;
      w.size++;
    }
    else
    {
      // write code for w
      int code = LZDictGetCode(dict, &w);
      LZWriteCode(dict, result_write, code);

      // add wK to dictionary
      LZDictAddEntry(dict, &w, K);

      // if dictionary is full, empty it out
      if (LZDictGetNumEntries(dict) >= MAX_ENTRIES)
      {
        LZDestroyDictionary(dict);
        dict = LZCreateDefaultDictionary();
      }

      // w = K
      w.data = (byte*)realloc(w.data, 1);
      w.data[0] = K;
      w.size = 1;
    }
  }

  // write code for w
  code = LZDictGetCode(dict, &w);
  LZWriteCode(dict, result_write, code);

  // clean up
  w.data = (byte*)realloc(w.data, 0);
  LZDestroyDictionary(dict);
  CloseBufferWrite(result_write);

  return result;
}

////////////////////////////////////////////////////////////////////////////////

static int LZReadCode(BUFFER_READ buffer_read)
{
  int code = 0;
  int i;
  for (i = 0; i < BITS; i++)
    code = (code << 1) + ReadBufferBit(buffer_read);
  return code;
}

////////////////////////////////////////////////////////////////////////////////

static void LZOutputCode(BUFFER_WRITE buffer_write, LZDICT dict, int code)
{
  LZBUFFER o = LZDictGetEntry(dict, code);
  int i;
  for (i = 0; i < o.size; i++)
    WriteBufferByte(buffer_write, o.data[i]);
}

////////////////////////////////////////////////////////////////////////////////

BUFFER LZDecode(BUFFER source)
{
  BUFFER result = CreateEmptyBuffer();
  int num_codes = (GetBufferSize(source) * 8) / 12;
  BUFFER_READ source_read = OpenBufferRead(source);
  BUFFER_WRITE result_write = OpenBufferWrite(result);
  LZDICT dict = LZCreateDefaultDictionary();

  int old_code;
  int new_code;
  LZBUFFER string = empty;

  // if the buffer is empty, don't do anything
  if (num_codes == 0)
    return result;

  old_code = LZReadCode(source_read);
  LZOutputCode(result_write, dict, old_code);

  while (--num_codes) // use prefix decrement because we're already read a code
  {
    int i;
    LZBUFFER o;

    new_code = LZReadCode(source_read);

    // if new_code is not in dictionary
    if (new_code >= LZDictGetNumEntries(dict))
    {
      // string = translation of old_code
      o = LZDictGetEntry(dict, old_code);
      string.size = o.size;
      string.data = (byte*)realloc(string.data, o.size + 1);
      memcpy(string.data, o.data, o.size);

      // string += first character in string
      string.data[string.size] = string.data[0];
      string.size++;
    }
    else
    {
      // string = translation of new_code
      o = LZDictGetEntry(dict, new_code);
      string.size = o.size;
      string.data = (byte*)realloc(string.data, o.size);
      memcpy(string.data, o.data, o.size);
    }

    // output string
    for (i = 0; i < string.size; i++)
      WriteBufferByte(result_write, string.data[i]);

    // CHARACTER = first character in STRING
    // add old_code + CHARACTER to dictionary
    // old_code = new_code
    o = LZDictGetEntry(dict, old_code);
    LZDictAddEntry(dict, &o, string.data[0]);
    old_code = new_code;

    // if dictionary is full, empty it out
    if (LZDictGetNumEntries(dict) >= MAX_ENTRIES)
    {
      LZDestroyDictionary(dict);
      dict = LZCreateDefaultDictionary();
    }
  }

  // clean up
  string.data = (byte*)realloc(string.data, 0);
  CloseBufferRead(source_read);
  CloseBufferWrite(result_write);
  LZDestroyDictionary(dict);

  return result;
}

////////////////////////////////////////////////////////////////////////////////
