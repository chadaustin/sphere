// huffman.c
// Chad Austin (aegis@nerv-un.net)
// 5.27.99
// Huffman Compression Routines


#include <stdlib.h>
#include <string.h>
#include "huffman.h"


struct _node;
typedef struct _node* nodeptr;


typedef struct _node
{
  bool isleaf;

  int  probability;
  byte character;

  nodeptr left;
  nodeptr right;
} node;

////////////////////////////////////////////////////////////////////////////////

static void swap(int* a, int* b)
{
  int c = *a;
  *a = *b;
  *b = c;
}

////////////////////////////////////////////////////////////////////////////////

static bool gcp(char* s, node* n, int c)
{
  if (n->isleaf == true)
  {
    if (c == n->character)
      return true;
    else
      return false;
  }
  else
  {
    char _s[257];
    if (gcp(s, n->left, c))
    {
      strcpy(_s, s);
      strcpy(s, "0");
      strcat(s, _s);
      return true;
    }
    else if (gcp(s, n->right, c))
    {
      strcpy(_s, s);
      strcpy(s, "1");
      strcat(s, _s);
      return true;
    }
    else
      return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

static char* get_character_path(node* n, int c)
{
  static char string[257];
  strcpy(string, "");
  if (gcp(string, n, c))
    return string;
  else
    return "";
}

////////////////////////////////////////////////////////////////////////////////

static void freetree(node* n)
{
  if (n->isleaf)
    free(n);
  else
  {
    freetree(n->left);
    freetree(n->right);
  }
}

////////////////////////////////////////////////////////////////////////////////

void writetree(BUFFER_WRITE buffer_write, node* n)
{
  if (n->isleaf)
  {
    WriteBufferByte(buffer_write, 1);
    WriteBufferByte(buffer_write, n->character);
  }
  else
  {
    WriteBufferByte(buffer_write, 0);
    writetree(buffer_write, n->left);
    writetree(buffer_write, n->right);
  }
}

////////////////////////////////////////////////////////////////////////////////

BUFFER HuffmanEncode(BUFFER source)
{
  int i;
  dword bytecount[256];

  int nodecount;
  node** nodes;

  char* strings[256];

  // create the result buffer and create a write handle
  BUFFER result = CreateEmptyBuffer();
  BUFFER_WRITE result_write = OpenBufferWrite(result);

  // clear frequency table
  for (i = 0; i < 256; i++)
    bytecount[i] = 0;

  // build frequency table
  for (i = 0; i < (int)GetBufferSize(source); i++)
    bytecount[ GetBufferData(source)[i] ]++;

  // build frequency forest
  nodecount = 0;
  nodes = NULL;
  for (i = 0; i < 256; i++)
    if (bytecount[i] > 0)
    {
      nodes = (node**)realloc(nodes, (nodecount + 1) * sizeof(node*));

      nodes[nodecount] = (node*)malloc(sizeof(node));
      nodes[nodecount]->isleaf = true;
      nodes[nodecount]->probability = bytecount[i];
      nodes[nodecount]->character = i;
      nodes[nodecount]->left = NULL;
      nodes[nodecount]->right = NULL;

      nodecount++;
    }

  // make sure forest has at least two trees
  while (nodecount < 2)
  {
    nodes = (node**)realloc(nodes, (nodecount + 1) * sizeof(node*));

    nodes[nodecount] = (node*)malloc(sizeof(node));
    nodes[nodecount]->isleaf = true;
    nodes[nodecount]->probability = 0;
    nodes[nodecount]->character = 0;
    nodes[nodecount]->left = NULL;
    nodes[nodecount]->right = NULL;

    nodecount++;
  }

  // generate huffman tree from forest (combine smallest trees until only one remains)
  while (nodecount > 1)
  {
    int minimum1, location1;
    int minimum2, location2;
    node* newnode;

    // find the first minimum value
    minimum1 = nodes[0]->probability;
    location1 = 0;
    for (i = 1; i < nodecount; i++)
      if (nodes[i]->probability < minimum1)
      {
        minimum1 = nodes[i]->probability;
        location1 = i;
      }

    // find the second minimum value (cannot be the same as first)
    if (location1 == 0)
    {
      minimum2 = nodes[1]->probability;
      location2 = 1;
    }
    else
    {
      minimum2 = nodes[0]->probability;
      location2 = 0;
    }

    for (i = 1; i < nodecount; i++)
      if (i != location1 && nodes[i]->probability < minimum2)
      {
        minimum2 = nodes[i]->probability;
        location2 = i;
      }

    // make sure location1 is always less than location2
    if (location2 < location1)
    {
      swap(&location2, &location1);
      swap(&minimum2, &minimum1);
    }

    // combine the two nodes
    newnode = (node*)malloc(sizeof(node));
    newnode->isleaf = false;
    newnode->probability = nodes[location1]->probability + nodes[location2]->probability;
    newnode->character = 0;
    newnode->left = nodes[location1];
    newnode->right = nodes[location2];
    
    // put combined nodes in first node
    nodes[location1] = newnode;

    // delete the node at location2
    for (i = location2; i < nodecount - 1; i++)
      nodes[i] = nodes[i + 1];
    nodecount--;
    nodes = (node**)realloc(nodes, nodecount * sizeof(node*));
  }

  // generate a map of strings for the characters
  for (i = 0; i < 256; i++)
  {
    strings[i] = (char*)malloc(257);
    strcpy(strings[i], get_character_path(nodes[0], i));
  }

  // output the buffer size
  WriteBufferDword(result_write, GetBufferSize(source));

  // output the huffman tree
  writetree(result_write, nodes[0]);

  // output the bits
  for (i = 0; i < (int)GetBufferSize(source); i++)
  {
    char* str = strings[GetBufferData(source)[i]];
    int j;
    for (j = 0; j < (int)strlen(str); j++)
      WriteBufferBit(result_write, (byte)(str[j] - '0'));
  }

  // close the writing functions
  CloseBufferWrite(result_write);

  // free the strings
  for (i = 0; i < 256; i++)
    free(strings[i]);

  // free the huffman tree
  freetree(nodes[0]);
  free(nodes);

  return result;
}

////////////////////////////////////////////////////////////////////////////////

static node* readtree(BUFFER_READ buffer_read)
{
  node* n;
  byte b;

  n = (node*)malloc(sizeof(node));

  b = ReadBufferByte(buffer_read);
  if (b == 0)         // it's a branch
  {
    n->isleaf = false;
    n->probability = 0;
    n->character = 0;
    n->left = readtree(buffer_read);
    n->right = readtree(buffer_read);
  }
  else                // it's a leaf
  {
    n->isleaf = true;
    n->probability = 0;
    n->character = ReadBufferByte(buffer_read);
    n->left = NULL;
    n->right = NULL;
  }
  return n;
}

////////////////////////////////////////////////////////////////////////////////

BUFFER HuffmanDecode(BUFFER source)
{
  dword buffersize;
  int decodedbytes;
  node* tree;
  node* curnode;

  // initialize buffer reading and writing functions
  BUFFER result             = CreateEmptyBuffer();
  BUFFER_READ source_read   = OpenBufferRead(source);
  BUFFER_WRITE result_write = OpenBufferWrite(result);

  // get size of destination buffer
  buffersize = ReadBufferDword(source_read);

  // read huffman tree
  tree = readtree(source_read);
 
  // decompress the buffer
  decodedbytes = 0;
  curnode = tree;
  while (decodedbytes < (int)buffersize)
  {
    if (ReadBufferBit(source_read) == 0) // take a left
      curnode = curnode->left;
    else                // take a right
      curnode = curnode->right;

    // if we're at a leaf, output the character and start over
    if (curnode->isleaf)
    {
      WriteBufferByte(result_write, curnode->character);
      decodedbytes++;
      curnode = tree;
    }
  }

  // free the tree
  freetree(tree);

  // close reading and writing routines
  CloseBufferRead(source_read);
  CloseBufferWrite(result_write);

  return result;
}

////////////////////////////////////////////////////////////////////////////////
