#ifndef EDITOR_HPP
#define EDITOR_HPP


#include <string>
#include <afxwin.h>
#include <afxext.h>


class CEditorApplication : public CWinApp
{
public:
  CEditorApplication();

private:
  BOOL InitInstance();
  int  ExitInstance();
};


extern std::string g_SphereDirectory;


extern void        SetStatusBar(CStatusBar* status_bar);
extern CStatusBar* GetStatusBar();


#endif
