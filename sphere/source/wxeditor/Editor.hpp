#ifndef EDITOR_HPP
#define EDITOR_HPP


#include <algorithm>
#include <string>
#include <wx/wx.h>
#include "MainWindow.hpp"


#if defined(_MSC_VER) && _MSC_VER <= 1200
namespace std {
  template<typename T>
  T min(T a, T b) {
    return _cpp_min(a, b);
  }

  template<typename T>
  T max(T a, T b) {
    return _cpp_max(a, b);
  }
}
#endif


class EditorApplication : public wxApp
{
public:
  EditorApplication();
  bool OnInit();
  int  OnExit();

protected:

private:
//  DECLARE_APP(EditorApplication)
};


extern std::string g_SphereDirectory;

extern void        SetStatusBar(wxStatusBar* status_bar);
extern wxStatusBar* GetStatusBar();
extern void SetStatus(const wxString &text, int i = 0);
extern void SetMainWindow(wMainWindow *mainwindow);
extern wMainWindow *GetMainWindow();

#endif
