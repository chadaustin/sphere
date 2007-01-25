#include "OptionsDialog.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include "resource.h"


BEGIN_MESSAGE_MAP(COptionsDialog, CDialog)

  ON_COMMAND(IDC_REGISTER_RMP,   OnRegisterMap)
  ON_COMMAND(IDC_REGISTER_RSS,   OnRegisterSpriteset)
  ON_COMMAND(IDC_REGISTER_RWS,   OnRegisterWindowStyle)
  ON_COMMAND(IDC_REGISTER_RFN,   OnRegisterFont)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

COptionsDialog::COptionsDialog()
: CDialog(IDD_OPTIONS)
{
}

////////////////////////////////////////////////////////////////////////////////

void
COptionsDialog::RegisterFileType(const char* extension, const char* filetype)
{
  // get the executable name
  CWinApp* app = AfxGetApp();
  char exename[513];
  GetModuleFileName(app->m_hInstance, exename, 512);

  HKEY key;

  // register the extension
  RegCreateKey(HKEY_CLASSES_ROOT, extension, &key);
  RegSetValue(key, NULL, REG_SZ, filetype, strlen(filetype));
  RegCloseKey(key);

  // register the type
  RegCreateKey(HKEY_CLASSES_ROOT, filetype, &key);
  RegSetValue(key, "shell\\open\\command", REG_SZ, "\"" + CString(exename) + "\" \"%1\"", strlen(exename) + 5);
  RegCloseKey(key);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg BOOL
COptionsDialog::OnInitDialog()
{
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
COptionsDialog::OnOK()
{
  CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////

#define REGISTER_HANDLER(name, ext, type)       \
afx_msg void                                    \
COptionsDialog::OnRegister##name()              \
{                                               \
  RegisterFileType(ext, type);                  \
}

REGISTER_HANDLER(Map,         ".rmp", "sdeMap");
REGISTER_HANDLER(Spriteset,   ".rss", "sdeSpriteset");
REGISTER_HANDLER(WindowStyle, ".rws", "sdeWindowStyle");
REGISTER_HANDLER(Font,        ".rfn", "sdeFont");

////////////////////////////////////////////////////////////////////////////////
