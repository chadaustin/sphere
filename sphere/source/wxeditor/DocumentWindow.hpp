#ifndef DOCUMENT_WINDOW_HPP
#define DOCUMENT_WINDOW_HPP


//#include <afxwin.h>
#include <vector>
#include <wx/wx.h>
#include <wx/docmdi.h>
/*todo:
const int WM_DW_CLOSING = (WM_APP + 1000);


const int WA_DOCUMENT_WINDOW = 0x0001;
const int WA_SAVEABLE = 0x0002;
*/

class wPaletteWindow;  // #include "PaletteWindow.hpp"


class wDocumentWindow : public wxMDIChildFrame
{
public:
  wDocumentWindow::wDocumentWindow(const char* document_path, int menu_resource, const wxSize& min_size = wxSize(0, 0));
  ~wDocumentWindow();

  virtual bool Close();

  const char* GetFilename() const;

  void AttachPalette(wPaletteWindow* palette);
  void DetachPalette(wPaletteWindow* palette);

  int GetNumPalettes() const;
  wPaletteWindow* GetPalette(int i) const;

  virtual bool IsSaveable() const;

  virtual void OnFileSave(wxCommandEvent &event);
  virtual void OnFileSaveAs(wxCommandEvent &event);
  virtual void OnFileSaveCopyAs(wxCommandEvent &event);

  void OnGetFocus(wxFocusEvent &event);
  void OnClose(wxCloseEvent &event);

protected:
  bool Create(char *class_name);
  static const char* GetDefaultWindowClass();
  
  void        SetDocumentPath(const char* path);
  const char* GetDocumentPath() const;
  virtual const char* GetDocumentTitle() const;

  void SetCaption(const char* caption);
  const char* GetCaption();
  virtual void UpdateWindowCaption();

/*todo:
  void OnCreate(LPCREATESTRUCT cs);
  void OnClose();
  void OnSizing(UINT side, LPRECT rect);
  void OnMDIActivate(BOOL activate, CWnd* active_window, CWnd* inactive_window);

  void OnUpdateSaveableCommand(CCmdUI* cmdui);
*/
private:
  char  m_DocumentPath[1024/*MAX_PATH*/];
  char  m_Caption[520];

  int   m_MenuResource;
  wxSize m_MinSize;

  std::vector<wPaletteWindow*> m_AttachedPalettes;

private:
//  DECLARE_CLASS(wDocumentWindow);
  DECLARE_EVENT_TABLE();
};


#endif
