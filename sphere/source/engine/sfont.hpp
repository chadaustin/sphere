#ifndef __SFONT_H
#define __SFONT_H


#include <vector>
#include "../common/Font.hpp"
#include "video.hpp"


class SFONT
{
public:
  SFONT();
  ~SFONT();

  bool Load(const char* filename, IFileSystem& fs);
  bool CreateFromFont(const sFont& font);

  bool DrawString(int x, int y, const char* text, RGBA mask) const;
  bool DrawZoomedString(int x, int y, double scale, const char* text, RGBA mask) const;
  bool DrawTextBox(int x, int y, int w, int h, int offset, const char* text, RGBA mask) const;

  int GetMaxHeight() const;
  int GetStringWidth(const char* string) const;
  int GetStringHeight(const char* string, int width) const;

private:
  bool Initialize();
  void Destroy();

private:
  sFont  m_Font;
  IMAGE* m_Images;
};


#endif
