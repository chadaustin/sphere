#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lzdict.h"


#define HASH_SIZE 1024


typedef struct _HASH_NODE
{
  LZBUFFER           buffer;
  int                code;
  struct _HASH_NODE* next;
} HASH_NODE;

typedef struct
{
  int        num_entries;
  HASH_NODE* hash_table[HASH_SIZE];
  LZBUFFER** code_indices;
} _LZDICT;


////////////////////////////////////////////////////////////////////////////////

LZDICT LZCreateDictionary(int max_entries)
{
  _LZDICT* _dict = (_LZDICT*)malloc(sizeof(_LZDICT));
  int i;
  
  _dict->num_entries = 0;

  // initialize hash table
  for (i = 0; i < HASH_SIZE; i++)
    _dict->hash_table[i] = NULL;

  // allocate code indices
  _dict->code_indices = (LZBUFFER**)malloc(max_entries * sizeof(LZBUFFER*));

  return _dict;
}

////////////////////////////////////////////////////////////////////////////////

static void LZFreeChain(HASH_NODE* node)
{
  if (node == NULL)
    return;

  LZFreeChain(node->next);
  free(node->buffer.data);
  free(node);
}

////////////////////////////////////////////////////////////////////////////////

void LZDestroyDictionary(LZDICT dict)
{
  _LZDICT* _dict = (_LZDICT*)dict;
  int i;

  // free hash table
  for (i = 0; i < HASH_SIZE; i++)
    LZFreeChain(_dict->hash_table[i]);

  free(_dict->code_indices);
  free(dict);
}

////////////////////////////////////////////////////////////////////////////////

int LZDictGetNumEntries(LZDICT dict)
{
  _LZDICT* _dict = (_LZDICT*)dict;
  return _dict->num_entries;
}

////////////////////////////////////////////////////////////////////////////////

void LZDictAddEntry(LZDICT dict, LZBUFFER* buffer, byte next)
{
  _LZDICT* _dict = (_LZDICT*)dict;
  int sum = 0;
  int i;
  HASH_NODE** p;

  // do hashing function
  for (i = 0; i < buffer->size; i++)
    sum += buffer->data[i];
  sum += next;
  sum %= HASH_SIZE;

  // find the bottom of the chain
  p = _dict->hash_table + sum;
  while (*p)
    p = &(*p)->next;

  // allocate a new node and stick it on the bottom of the chain
  *p = (HASH_NODE*)malloc(sizeof(HASH_NODE));
  (*p)->buffer.size = buffer->size + 1;
  (*p)->buffer.data = (byte*)malloc(buffer->size + 1);
  memcpy((*p)->buffer.data, buffer->data, buffer->size);
  (*p)->buffer.data[buffer->size] = next;
  (*p)->code = _dict->num_entries;
  (*p)->next = NULL;

  _dict->code_indices[_dict->num_entries] = &((*p)->buffer);
  _dict->num_entries++;
}

////////////////////////////////////////////////////////////////////////////////

bool LZDictContainsEntry(LZDICT dict, LZBUFFER* buffer, byte next)
{
  _LZDICT* _dict = (_LZDICT*)dict;
  int sum = 0;
  int i;
  HASH_NODE* p;

  // do hashing function
  for (i = 0; i < buffer->size; i++)
    sum += buffer->data[i];
  sum += next;
  sum %= HASH_SIZE;

  // step down the chain looking for the entry
  p = _dict->hash_table[sum];
  while (p)
  {
    if (p->buffer.size == buffer->size + 1 &&
        memcmp(p->buffer.data, buffer->data, buffer->size) == 0 &&
        p->buffer.data[buffer->size] == next)
      return true;
    p = p->next;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

int LZDictGetCode(LZDICT dict, LZBUFFER* buffer)
{
  _LZDICT* _dict = (_LZDICT*)dict;
  int sum = 0;
  int i;
  HASH_NODE* p;

  // do hashing function
  for (i = 0; i < buffer->size; i++)
    sum += buffer->data[i];
  sum %= HASH_SIZE;

  // step down the chain and test if the code is in it
  p = _dict->hash_table[sum];
  while (p)
  {
    if (p->buffer.size == buffer->size &&
        memcmp(p->buffer.data, buffer->data, buffer->size) == 0)
      return p->code;
    p = p->next;
  }

  return -1;
}

////////////////////////////////////////////////////////////////////////////////

LZBUFFER LZDictGetEntry(LZDICT dict, int code)
{
  _LZDICT* _dict = (_LZDICT*)dict;
  assert(code >= 0 && code < _dict->num_entries);
  return *(_dict->code_indices[code]);
}

////////////////////////////////////////////////////////////////////////////////
