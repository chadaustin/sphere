#ifndef SAVEABLE_DOCUMENT_WINDOW_HPP
#define SAVEABLE_DOCUMENT_WINDOW_HPP


#include "DocumentWindow.hpp"


class CSaveableDocumentWindow : public CDocumentWindow
{
public:
  CSaveableDocumentWindow(const char* document_path, int menu_resource, const CSize& min_size = CSize(0, 0));
  ~CSaveableDocumentWindow();

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

  virtual void ImageToolBarChanged(UINT id);
  virtual void MapToolBarChanged(UINT id);
  virtual void UpdateToolBars();

  afx_msg void OnFileSave();
  afx_msg void OnFileSaveAs();
  afx_msg void OnFileSaveCopyAs();

private:
  void UpdateProject();

  virtual bool GetSavePath(char* path) = 0;
  virtual bool SaveDocument(const char* path) = 0;

private:
  bool m_Saved;
  bool m_Modified;

  DECLARE_MESSAGE_MAP()
};


#endif
