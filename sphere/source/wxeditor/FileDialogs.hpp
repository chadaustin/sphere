#ifndef FILE_DIALOGS_HPP
#define FILE_DIALOGS_HPP


// identifier too long
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include <string>
//#include <afxdlgs.h>
#include <wx/filedlg.h>
#include "FileTypes.hpp"


enum EFileDialogMode {
  FDM_OPEN        = 0x01,
  FDM_SAVE        = 0x02,
  FDM_MAYNOTEXIST = 0x04,
  FDM_MULTISELECT = 0x08,
};


// fix VC++ internal compiler error  ;_;
// can't just pass integers into the template class

#define MAKE_FILETYPE(type, i)  \
  struct type {                 \
    enum { ft = i };            \
  }; 

  MAKE_FILETYPE(FDT_MAPS,         GT_MAPS)
  MAKE_FILETYPE(FDT_TILESETS,     GT_TILESETS)
  MAKE_FILETYPE(FDT_SPRITESETS,   GT_SPRITESETS)
  MAKE_FILETYPE(FDT_SCRIPTS,      GT_SCRIPTS)
  MAKE_FILETYPE(FDT_SOUNDS,       GT_SOUNDS)
  MAKE_FILETYPE(FDT_FONTS,        GT_FONTS)
  MAKE_FILETYPE(FDT_WINDOWSTYLES, GT_WINDOWSTYLES)
  MAKE_FILETYPE(FDT_IMAGES,       GT_IMAGES)
  MAKE_FILETYPE(FDT_ANIMATIONS,   GT_ANIMATIONS)
  MAKE_FILETYPE(FDT_PACKAGES,     GT_PACKAGES)

#undef MAKE_FILETYPE


template<typename filetype>
class wSphereFileDialog : public wxFileDialog
{
public:
  wSphereFileDialog(wxWindow *parent, int mode, const char* title = NULL) 
  : wxFileDialog(parent,
      title == NULL ? 
          mode & FDM_OPEN ? wxString("Open") : wxString("Save As") :
          wxString(title),
      ::wxGetCwd(),
      wxString(""),
      GenerateFilter(mode),
      mode & FDM_OPEN ? wxOPEN : wxSAVE
    )
  {
/*
      mode & FDM_OPEN ? TRUE : FALSE,
      FTL.GetDefaultExtension(filetype::ft),
      NULL,
      OFN_HIDEREADONLY |
        (mode & FDM_OPEN && !(mode & FDM_MAYNOTEXIST) ? OFN_FILEMUSTEXIST : 0) |
        (mode & FDM_MULTISELECT ? OFN_ALLOWMULTISELECT : 0),
      GenerateFilter(mode),
      NULL
    )
    if (title) {
      m_ofn.lpstrTitle = title;
    }
*/
  }

  ~wSphereFileDialog()
  {
    delete[] m_filter;
  }

  virtual int DoModal()
  {
    // set the current directory to be the initial one
    //char path[MAX_PATH];
    //GetCurrentDirectory(MAX_PATH, path);
    //m_ofn.lpstrInitialDir = path;

    return wxFileDialog::ShowModal();
  }

private:
  const char* GenerateFilter(int mode)
  {
    // generate a filter with all supported types
    std::string filter = GenerateOverallFilter(mode);
    
    // generate filters for all of the subtypes    
    if (mode & FDM_OPEN && FTL.GetNumSubTypes(filetype::ft) != 1) {
      for (int i = 0; i < FTL.GetNumSubTypes(filetype::ft); i++) {
        filter += GenerateSubTypeFilter(i);
      }
    }

    filter += "All Files (*.*)|*.*||";
    
    m_filter = new char[filter.length() + 1];
    strcpy(m_filter, filter.c_str());
    return m_filter;
  }


  static std::string GenerateOverallFilter(int mode)
  {
    // get all extensions
    std::vector<std::string> extensions;
    if (mode & FDM_OPEN) { 
      FTL.GetFileTypeExtensions(filetype::ft, extensions);
    } else {
      extensions.push_back(FTL.GetDefaultExtension(filetype::ft));
    }

    // make a semicolon-separated string
    std::string filters;
    for (unsigned i = 0; i < extensions.size(); i++) {
      filters += "*." + extensions[i];
      if (i < extensions.size() - 1) {
        filters += ";";
      }
    }

    std::string filter = FTL.GetFileTypeLabel(filetype::ft);
    filter += " (" + filters + ")";
    filter += "|" + filters + "|";
    return filter;
  }


  static std::string GenerateSubTypeFilter(int sub_type)
  {
    // get all extensions
    std::vector<std::string> extensions;
    FTL.GetSubTypeExtensions(filetype::ft, sub_type, extensions);

    // make a semicolon-separated string
    std::string filters;
    for (unsigned i = 0; i < extensions.size(); i++) {
      filters += "*." + extensions[i];
      if (i < extensions.size() - 1) {
        filters += ";";
      }
    }

    std::string filter = FTL.GetSubTypeLabel(filetype::ft, sub_type);
    filter += " (" + filters + ")";
    filter += "|" + filters + "|";
    return filter;
  }

private:
  char* m_filter;
};


#define DEFINE_FILE_DIALOG(name, filetype) \
typedef wSphereFileDialog<filetype> name;

  DEFINE_FILE_DIALOG(wMapFileDialog,         FDT_MAPS)
  DEFINE_FILE_DIALOG(wTilesetFileDialog,     FDT_TILESETS)
  DEFINE_FILE_DIALOG(wSpritesetFileDialog,   FDT_SPRITESETS)
  DEFINE_FILE_DIALOG(wSoundFileDialog,       FDT_SOUNDS)
  DEFINE_FILE_DIALOG(wScriptFileDialog,      FDT_SCRIPTS)
  DEFINE_FILE_DIALOG(wImageFileDialog,       FDT_IMAGES)
  DEFINE_FILE_DIALOG(wAnimationFileDialog,   FDT_ANIMATIONS)
  DEFINE_FILE_DIALOG(wWindowStyleFileDialog, FDT_WINDOWSTYLES)
  DEFINE_FILE_DIALOG(wFontFileDialog,        FDT_FONTS)
  DEFINE_FILE_DIALOG(wPackageFileDialog,     FDT_PACKAGES)

#undef DEFINE_FILE_DIALOG


#endif
