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
  bool DrawWindow(int x, int y, int w, int h);

private:
  void Initialize();
  void Destroy();

private:
  sWindowStyle m_WindowStyle;
  IMAGE m_Images[9];
};


#endif
