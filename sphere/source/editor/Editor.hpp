#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <string>
#include <afxwin.h>
#include <afxext.h>
#include "MainWindow.hpp"


class CInstanceRepository;
class CEditorApplication : public CWinApp
{
public:
  CEditorApplication();

#ifdef MIRC_LIKE_THING
  void OnInitialUpdate();
#endif

private:
  BOOL InitInstance();
  int  ExitInstance();	
	CInstanceRepository* m_Instances;
};


extern std::string GetSphereDirectory();

extern void        SetStatusBar(CStatusBar* status_bar);
extern CStatusBar* GetStatusBar();

extern CMainWindow* GetMainWindow();

const char* TranslateString(const char* string);
void TranslateMenu(HMENU menu);

const char* GetLanguage();
void SetLanguage(const char* language);

#endif
