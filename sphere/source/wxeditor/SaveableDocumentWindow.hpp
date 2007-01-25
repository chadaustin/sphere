#ifndef SAVEABLE_DOCUMENT_WINDOW_HPP
#define SAVEABLE_DOCUMENT_WINDOW_HPP


#include "DocumentWindow.hpp"


class wSaveableDocumentWindow : public wDocumentWindow
{
public:
  wSaveableDocumentWindow(const char* document_path, int menu_resource, const wxSize& min_size = wxSize(64, 64));
  ~wSaveableDocumentWindow();

  bool Save();
  virtual bool Close();

  bool IsSaved() const;
  bool IsModified() const;

protected:
  void SetSaved(bool saved);
  void SetModified(bool modified);

  virtual const char* GetDocumentTitle() const;
  virtual void UpdateWindowCaption();

  virtual bool IsSaveable() const;

  virtual void OnFileSave(wxCommandEvent &event);
  virtual void OnFileSaveAs(wxCommandEvent &event);
  virtual void OnFileSaveCopyAs(wxCommandEvent &event);

private:
  void UpdateProject();

  virtual bool GetSavePath(char* path) = 0;
  virtual bool SaveDocument(const char* path) = 0;

private:
  bool m_Saved;
  bool m_Modified;

private:
//  DECLARE_CLASS(wSaveableDocumentWindow)
  DECLARE_EVENT_TABLE()
};


#endif
