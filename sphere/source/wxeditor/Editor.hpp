#ifndef EDITOR_HPP
#define EDITOR_HPP


#include <string>
//#include <afxwin.h>
//#include <afxext.h>
#include <wx/wx.h>
#include "MainWindow.hpp"


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
extern void SetStatus(wxString &text, int i = 0);
extern void SetMainWindow(wMainWindow *mainwindow);
extern wMainWindow *GetMainWindow();

#endif
