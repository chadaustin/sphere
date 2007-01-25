#ifndef ANIMATION_WINDOW_HPP
#define ANIMATION_WINDOW_HPP


#include <wx/wx.h>
#include "DocumentWindow.hpp"
#include "../common/IAnimation.hpp"
#include "../common/Image32.hpp"


class CFlicAnimation; // #include "flic.hpp"
class wDIBSection;    // #include "DIBSection.hpp"


class wAnimationWindow : public wDocumentWindow
{
public:
  wAnimationWindow(const char* animation);
  ~wAnimationWindow();

private:
  //void OnDestroy();
  void OnPaint(wxPaintEvent &event);
  void OnTimer(wxTimerEvent &event);

private:
  IAnimation*  m_Animation;
  wDIBSection* m_BlitFrame;
  CImage32*    m_ImageFrame;

  int m_Delay;
  wxTimer *m_Timer;

private:
  //DECLARE_CLASS(wAnimationWindow)
  DECLARE_EVENT_TABLE()
};


#endif
