#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "i32.h"
#include "huffman.h"
#include "lz.h"


////////////////////////////////////////////////////////////////////////////////

bool AllocImage32(IMAGE32* image, int width, int height)
{
  image->data = (RGBA*)malloc(width * height * 4);
  if (image->data == NULL) return false;
  image->width = width;
  image->height = height;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool FreeImage32(IMAGE32* image)
{
  image->width = 0;
  image->height = 0;
  free(image->data);
  image->data = NULL;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

static int bytesleft(FILE* file)
{
  int currentlocation = ftell(file);
  int filesize;
  fseek(file, 0, SEEK_END);
  filesize = ftell(file);
  fseek(file, currentlocation, SEEK_SET);
  return filesize - currentlocation;
}

////////////////////////////////////////////////////////////////////////////////

bool LoadImage32(IMAGE32* image, const char *filename)
{
  FILE* file;

  file = fopen(filename, "rb");
  if (file == NULL)
    return false;

  // Reads image header
  fread(image, 1, 256, file);

  // Tests header
  if (memcmp(image->signature, ".i32", 4) != 0)
    { fclose(file); return false; }
  if (image->version != 1)
    { fclose(file); return false; }

  // Allocates image
  if (AllocImage32(image, image->width, image->height) == false)
    { fclose(file); return false; }

  if (image->compression == 0)  // no compression
  {
    // Reads data from file
    fread(image->data, 4, image->width * image->height, file);
  }
  else                          // huffman compression
  {
    int left = bytesleft(file);
    BUFFER source = CreateSizedBuffer(left);
    BUFFER result;
    fread(GetBufferData(source), 1, left, file);

    if (image->compression == 1)      // huffman
      result = HuffmanDecode(source);
    else if (image->compression == 2) // LZ
      result = LZDecode(source);

    memcpy(image->data, GetBufferData(result), GetBufferSize(result));

    DestroyBuffer(source);
    DestroyBuffer(result);
  }

  fclose(file);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool SaveImage32(IMAGE32* image, const char* filename, int compression)
{
  FILE* file;

  file = fopen(filename, "wb");
  if (file == NULL) return false;

  // Writes image header
  memcpy(image->signature, ".i32", 4);
  image->version = 1;
  image->compression = compression;
  fwrite(image, 1, 256, file);

  // Writes data to file
  if (compression == 0)
  {
    fwrite(image->data, 1, 4 * image->width * image->height, file);
  }
  else
  {
    BUFFER source = CreateBuffer(4 * image->width * image->height, (byte*)image->data);
    BUFFER result;

    if (compression == 1)
      result = HuffmanEncode(source);
    else
      result = LZEncode(source);

    fwrite(GetBufferData(result), 1, GetBufferSize(result), file);

    DestroyBuffer(source);
    DestroyBuffer(result);
  }

  fclose(file);
  return true;
}

////////////////////////////////////////////////////////////////////////////////
