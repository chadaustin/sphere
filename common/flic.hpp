#ifndef __FLIC_H
#define __FLIC_H



#include <stdio.h>
#include "types.h"
#include "colorspace.h"



class CFlicAnimation
{
public:
  CFlicAnimation(const char* FileName);
  ~CFlicAnimation();

  bool IsLoaded() const;

  int GetWidth() const;
  int GetHeight() const;
  int GetNumFrames() const;
  int GetDelay() const;

  bool ReadNextFrame(RGBA* FrameBuffer);
  bool ReadNextFrame(BGR*  FrameBuffer);

private:
  bool ReadFrame();

  bool DecodeChunk_COLOR256();
  bool DecodeChunk_SS2();
  bool DecodeChunk_COLOR();
  bool DecodeChunk_LC();
  bool DecodeChunk_BLACK();
  bool DecodeChunk_BRUN();
  bool DecodeChunk_COPY();

  byte  next_byte();
  sbyte next_sbyte();
  word  next_word();
  dword next_dword();
  RGB   next_RGB();
  void  read_next(byte* buffer, int numbytes);

private:
  bool  Loaded;

  int   Width;
  int   Height;
  int   Delay;

  int   CurrentFrame;
  int   NumFrames;

  byte* Frame;
  RGBA  Palette[256]; // alpha is unused

  FILE* File;
  int   NextFrame;    // position in file

  int   SecondFrame;  // position in file, used for looping

  byte* FrameData;
  int   FrameDataOffset;  // current location
};



#endif
