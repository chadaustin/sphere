#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include "sfont.hpp"


////////////////////////////////////////////////////////////////////////////////

SFONT::SFONT()
{
  m_Images = NULL;
}

////////////////////////////////////////////////////////////////////////////////

SFONT::~SFONT()
{
  Destroy();
}

////////////////////////////////////////////////////////////////////////////////

bool
SFONT::Load(const char* filename, IFileSystem& fs)
{
  // load file
  if (m_Font.Load(filename, fs) == false) {
    return false;
  }

  return Initialize();
}

////////////////////////////////////////////////////////////////////////////////

bool
SFONT::CreateFromFont(const sFont& font)
{
  m_Font = font;
  return Initialize();
}

////////////////////////////////////////////////////////////////////////////////

void
SFONT::Destroy()
{
  if (m_Images) {
    for (int i = 0; i < m_Font.GetNumCharacters(); i++) {
      if (m_Images[i]) {
        DestroyImage(m_Images[i]);
      }
    }
  }
  delete[] m_Images;
  m_Images = NULL;
}

////////////////////////////////////////////////////////////////////////////////

bool
SFONT::DrawString(int x, int y, const char* text, RGBA mask) const
{
  int range = m_Font.GetNumCharacters();

  while (*text) {
    char c = *text;
    if (c < 0 || c >= range) c = 0;

    const sFontCharacter& character = m_Font.GetCharacter(c);
    BlitImageMask(m_Images[c], x, y, mask);
    x += character.GetWidth();
    text++;
  }
  return true; 
}

////////////////////////////////////////////////////////////////////////////////

bool
SFONT::DrawZoomedString(int x, int y, double scale, const char* text, RGBA mask) const
{
  struct Local
  {
    static inline void ScaleBlit(IMAGE i, int x, int y, double scale, RGBA mask) {
      int w = GetImageWidth(i);
      int h = GetImageHeight(i);
      int ax[4] = { x, x + (int)(scale * w), x + (int)(scale * w), x };
      int ay[4] = { y, y, y + (int)(scale * h), y + (int)(scale * h) };
      TransformBlitImageMask(i, ax, ay, mask);
    }
  };

  double cx = x;
  int range = m_Font.GetNumCharacters();

  while (*text) {
    char c = *text;
    if (c < 0 || c >= range) c = 0;
    
    Local::ScaleBlit(m_Images[c], int(cx), y, scale, mask);

    cx += scale * m_Font.GetCharacter(c).GetWidth();
    text++;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
SFONT::DrawTextBox(int x, int y, int w, int h, int offset, const char* text, RGBA mask) const
{
  const int space_width = GetStringWidth(" ");
  const int tab_width   = GetStringWidth("   ");
  const int max_height  = GetMaxHeight();

  const char* p = text;

  int old_x, old_y, old_w, old_h;
  GetClippingRectangle(&old_x, &old_y, &old_w, &old_h);
  SetClippingRectangle(x, y, w, h);

  y += offset;

  // delta x and y from starting position
  int dx = 0;
  int dy = 0;

  std::string word;
  int word_width = 0;

  // parse the text into words
  while (*p) {

    if (*p == ' ') {          // if it's a space, draw the word
      
      if (dx + word_width + space_width > w) {
        dx = word_width + space_width;
        dy += max_height;

        DrawString(x, y + dy, word.c_str(), mask);
      } else {
        DrawString(x + dx, y + dy, word.c_str(), mask);
        dx += word_width + space_width;
      }

      word.resize(0);
      word_width = 0;

    } else if (*p == '\t') {  // if it's a tab, draw the word

      if (dx + word_width + tab_width > w) {
        dx = word_width + tab_width;
        dy += max_height;

        DrawString(x, y + dy, word.c_str(), mask);
      } else {
        DrawString(x + dx, y + dy, word.c_str(), mask);
        dx += word_width + tab_width;
      }

      word.resize(0);
      word_width = 0;

    } else if (*p == '\n') {  // newline time, awww yeah

      DrawString(x + dx, y + dy, word.c_str(), mask);
      dx = 0;
      dy += max_height;
      word.resize(0);
      word_width = 0;

    } else {

      int char_width = m_Font.GetCharacter(*p).GetWidth();

      // if we've gone over the limit and dx = 0, draw the old word and split the new one off
      if (word_width + char_width > w && dx == 0) {
        DrawString(x + dx, y + dy, word.c_str(), mask);
        dy += max_height;
        word.resize(0);
        word_width = 0;
      } else if (dx + word_width + char_width > w) {
        dx = 0;
        dy += max_height;
      }

      word += *p;
      word_width += char_width;

    }

    p++;
  }

  DrawString(x + dx, y + dy, word.c_str(), mask);


  SetClippingRectangle(old_x, old_y, old_w, old_h);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

int
SFONT::GetMaxHeight() const
{
  int max_height = 0;
  for (int i = 0; i < m_Font.GetNumCharacters(); i++)
    if (m_Font.GetCharacter(i).GetHeight() > max_height)
      max_height = m_Font.GetCharacter(i).GetHeight();
  return max_height;
}

////////////////////////////////////////////////////////////////////////////////

int
SFONT::GetStringWidth(const char* string) const
{
  int width = 0;
  while (*string)
  {
    width += m_Font.GetCharacter(*string).GetWidth();
    string++;
  }
  return width;
}

////////////////////////////////////////////////////////////////////////////////

int
SFONT::GetStringHeight(const char* string, int width) const
{
  // no point doing massive loops if the user only wants
  // just the overall height of a non-wordwrapped string
  if (width == 0)

    return GetMaxHeight();

  const int space_width = GetStringWidth(" ");
  const int tab_width   = GetStringWidth("   ");
  const int max_height = GetMaxHeight();
  const char* p = string;
  int dx = 0;
  int dy = 0;

  std::string word;
  int word_width = 0;

  while (*p) {

    if (*p == ' ') {          // if it's a space, draw the word
      
      if (dx + word_width + space_width > width) {
        dx = word_width + space_width;
        dy += max_height;
      } else {
        dx += word_width + space_width;
      }

      word.resize(0);
      word_width = 0;

    } else if (*p == '\t') {  // if it's a tab, draw the word

      if (dx + word_width + tab_width > width) {
        dx = word_width + tab_width;
        dy += max_height;
      } else {
        dx += word_width + tab_width;
      }

      word_width = 0;

    } else if (*p == '\n') {  // newline time, awww yeah

      dx = 0;
      dy += max_height;
      word.resize(0);
      word_width = 0;

    } else {

      int char_width = m_Font.GetCharacter(*p).GetWidth();

      // if we've gone over the limit and dx = 0, draw the old word and split the new one off
      if (word_width + char_width > width && dx == 0) {
        dy += max_height;
        word.resize(0);
        word_width = 0;
      } else if (dx + word_width + char_width > width) {
        dx = 0;
        dy += max_height;
      }

      word += *p;
      word_width += char_width;

    }

    p++;
  }

  // add a line more to give the total line height since this
  // algorithm was supposed to draw text!
  dy += max_height;
  return dy;
}

////////////////////////////////////////////////////////////////////////////////

bool
SFONT::Initialize()
{
  m_Images = new IMAGE[m_Font.GetNumCharacters()];
  for (int i = 0; i < m_Font.GetNumCharacters(); i++) {

    sFontCharacter& c = m_Font.GetCharacter(i);
    m_Images[i] = CreateImage(c.GetWidth(), c.GetHeight(), c.GetPixels());

    if (m_Images[i] == NULL) {
      for (int j = 0; j < i; j++) {
        DestroyImage(m_Images[j]);
      }
      return false;
    }

  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
