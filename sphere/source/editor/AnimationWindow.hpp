#ifndef ANIMATION_WINDOW_HPP
#define ANIMATION_WINDOW_HPP


#include "DocumentWindow.hpp"
#include "../common/IAnimation.hpp"


class CFlicAnimation; // #include "flic.hpp"
class CDIBSection;    // #include "DIBSection.hpp"


class CAnimationWindow : public CDocumentWindow
{
public:
  CAnimationWindow(const char* animation);
  ~CAnimationWindow();

private:
  afx_msg void OnDestroy();
  afx_msg void OnPaint();
  afx_msg void OnTimer(UINT event);

private:
  IAnimation*  m_Animation;
  CDIBSection* m_BlitFrame;

  int m_Delay;
  UINT m_Timer;

  DECLARE_MESSAGE_MAP()
};


#endif
