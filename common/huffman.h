// huffman.h
// Chad Austin (aegis@nerv-un.net)
// 5.30.99
// Interface to Huffman Compression Module


#ifndef __HUFFMAN_H
#define __HUFFMAN_H


#include "buffer.h"


#include "begin_c_prototypes.h"

  extern BUFFER HuffmanEncode(BUFFER source);
  extern BUFFER HuffmanDecode(BUFFER source);

#include "end_c_prototypes.h"


#endif
