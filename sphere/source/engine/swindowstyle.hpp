#ifndef SWINDOWSTYLE_HPP
#define SWINDOWSTYLE_HPP


#include "video.hpp"
#include "../common/WindowStyle.hpp"
#include "../common/IFileSystem.hpp"


class SWINDOWSTYLE
{
public:
  SWINDOWSTYLE();
  ~SWINDOWSTYLE();

  bool Load(const char* filename, IFileSystem& fs);
  bool CreateFromWindowStyle(const sWindowStyle& ws);
  void Destroy();

  int GetBackgroundMode();
  bool DrawMiddle(int x, int y, int w, int h, int background_mode);

  bool DrawTopEdge   (int x, int y, int w, int h);
  bool DrawBottomEdge(int x, int y, int w, int h);
  bool DrawLeftEdge  (int x, int y, int w, int h);
  bool DrawRightEdge (int x, int y, int w, int h);

  bool DrawUpperLeftCorner (int x, int y);
  bool DrawUpperRightCorner(int x, int y);
  bool DrawLowerLeftCorner (int x, int y);
  bool DrawLowerRightCorner(int x, int y);

  bool DrawWindow(int x, int y, int w, int h);

private:
  void Initialize();

private:
  sWindowStyle m_WindowStyle;
  IMAGE m_Images[9];
};


#endif
