#pragma warning(disable: 4786)

#include "OptionsDialog.hpp"
#include "resource.h"
#include "FileTypes.hpp"
#include "Editor.hpp"

#include <afxwin.h>
#include <afxcmn.h>

///////////////////////////////////////////////////////////

static
const char* __getfiletype__(const char* ext)
{
  const char* filetype = "";

  if (strcmp(ext, "rmp") == 0) filetype = "SphereMap";
  if (strcmp(ext, "rss") == 0) filetype = "SphereSpriteset";
  if (strcmp(ext, "rts") == 0) filetype = "SphereTileset";
  if (strcmp(ext, "rws") == 0) filetype = "SphereWindowStyle";
  if (strcmp(ext, "rfn") == 0) filetype = "SphereFont";
  if (strcmp(ext, "spk") == 0) filetype = "SpherePackage";
  if (strcmp(ext, "sgm") == 0) filetype = "SphereGame";

  return filetype;
}

static
bool IsRegistered(const char* ext)
{
  const char* filetype = __getfiletype__(ext);

  char extension[1024] = {0};
  sprintf (extension, ".%s", ext);

  if (1) {
    HKEY key;

    int retval = RegOpenKey(HKEY_CLASSES_ROOT, extension, &key);
    RegCloseKey(key);
    
    if (retval == 0) {
      retval = RegOpenKey(HKEY_CLASSES_ROOT, filetype, &key);
      RegCloseKey(key);

      if (retval == 0) {
        return true;
      }
    }
  }

  return false;
}

static
void RegisterFile(const char* ext)
{
  char sphere_editor[MAX_PATH];
  GetModuleFileName(AfxGetApp()->m_hInstance, sphere_editor, MAX_PATH);
  std::string sphere_engine = GetSphereDirectory() + "\\engine.exe";

  char extension[1024] = {0};
  sprintf (extension, ".%s", ext);

  const char* filetype = __getfiletype__(ext);

  if (1)
  {
    HKEY key;
    char command[MAX_PATH + 1024];
    sprintf (command, "\"%s\" \"%%1\"", sphere_editor);

    // register the extension
    RegCreateKey(HKEY_CLASSES_ROOT, extension, &key);
    RegSetValue(key, NULL, REG_SZ, filetype, strlen(filetype));
    RegCloseKey(key);

    // register the type
    RegCreateKey(HKEY_CLASSES_ROOT, filetype, &key);
    RegSetValue(key, "shell\\open\\command", REG_SZ, command, strlen(command));
    RegCloseKey(key);
  }
}

static
void UnregisterFile(const char* ext)
{

}

///////////////////////////////////////////////////////////

static
BOOL CALLBACK FileTypeDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  struct Local {
    static void RefreshFileTypes(HWND window, bool first) {

      // add filetypes
      int index = 0;
      for (int filetype = 0; filetype < FTL.GetNumFileTypes(); filetype++) {
        std::vector<std::string> extensions;
        FTL.GetFileTypeExtensions(filetype, false, extensions);
        for (unsigned int i = 0; i < extensions.size(); i++, index++) {
          if (first) SendDlgItemMessage(window, IDC_FILETYPE_LIST, LB_ADDSTRING, 0, (LPARAM)extensions[i].c_str());
          SendDlgItemMessage(window, IDC_FILETYPE_LIST, (UINT) LB_SETSEL, (WPARAM) IsRegistered(extensions[i].c_str()) ? TRUE : FALSE, (LPARAM)index);
        }
      }

      if (SendDlgItemMessage(window, IDC_FILETYPE_LIST, LB_GETCOUNT, 0, 0) <= 0) {
        EnableWindow(GetDlgItem(window, IDC_FILETYPES_REGISTER_TYPES), FALSE);
        EnableWindow(GetDlgItem(window, IDC_FILETYPES_UNREGISTER_TYPES), FALSE);
      }
    }

    static void SelectedFileTypes(HWND window, void (*SelectedFileTypeOperation)(const char* ext))
    {
      for (int i = 0; i < SendDlgItemMessage(window, IDC_FILETYPE_LIST, LB_GETCOUNT, 0, 0); i++) {
        int selected = SendDlgItemMessage(window, IDC_FILETYPE_LIST, (UINT) LB_GETSEL, (WPARAM) i, 0);
        if (selected > 0) {
          int len = SendDlgItemMessage(window, IDC_FILETYPE_LIST, LB_GETTEXTLEN, i, 0);
          if (len > 0 && len < 1020) {
            char text[1024];
            SendDlgItemMessage(window, IDC_FILETYPE_LIST, LB_GETTEXT, i, (WPARAM)text);
            text[len] = '\0';
            SelectedFileTypeOperation(text);
          }
        }
      }
    }
  };

  switch (message)
  {
    case WM_INITDIALOG:
    {
      Local::RefreshFileTypes(window, true);
      SetFocus(GetDlgItem(window, IDC_FILETYPE_LIST));
      return TRUE;
    }

    case WM_COMMAND:
    {
      if (LOWORD(wparam) == IDC_FILETYPE_LIST && HIWORD(wparam) == LBN_SELCHANGE)
      {
        if (SendDlgItemMessage(window, IDC_FILETYPE_LIST, LB_GETCOUNT, 0, 0) >= 1) {
          EnableWindow(GetDlgItem(window, IDC_FILETYPES_REGISTER_TYPES), TRUE);
          EnableWindow(GetDlgItem(window, IDC_FILETYPES_UNREGISTER_TYPES), TRUE);
        }
        return TRUE;
      }

      if (LOWORD(wparam) == IDC_FILETYPES_RESTORE_DEFAULTS)
      {
        for (int filetype = 0; filetype < FTL.GetNumFileTypes(); filetype++) {
          std::vector<std::string> extensions;
          FTL.GetFileTypeExtensions(filetype, false, extensions);
          for (unsigned int i = 0; i < extensions.size(); i++) {
            UnregisterFile(extensions[i].c_str());
          }
        }

        RegisterFile("sgm");
        RegisterFile("rmp");
        RegisterFile("rss");
        RegisterFile("rws");
        RegisterFile("rfn");
        RegisterFile("rts");
        RegisterFile("spk");
        Local::RefreshFileTypes(window, false);

        return TRUE;
      }

      if (LOWORD(wparam) == IDC_FILETYPES_REGISTER_TYPES)
      {
        Local::SelectedFileTypes(window, RegisterFile);
        return TRUE;
      }

      if (LOWORD(wparam) == IDC_FILETYPES_UNREGISTER_TYPES)
      {
        Local::SelectedFileTypes(window, UnregisterFile);
        return TRUE;
      }

      return FALSE;
    }

    default:
      return FALSE;
  }
}

///////////////////////////////////////////////////////////

void
COptionsDialog::Execute()
{
  PROPSHEETPAGE Pages[1];
  const int num_pages = sizeof(Pages) / sizeof(*Pages);

  // default values
  for (unsigned i = 0; i < num_pages; i++)
  {
    Pages[i].dwSize    = sizeof(Pages[i]);
    Pages[i].dwFlags   = PSP_DEFAULT;
    Pages[i].hInstance = AfxGetApp()->m_hInstance;
    Pages[i].hIcon     = NULL;
  }
  
  int current_page = 0;

  Pages[current_page].pszTemplate = MAKEINTRESOURCE(IDD_FILETYPES_PAGE);
  Pages[current_page].pfnDlgProc  = FileTypeDialogProc;
  current_page += 1;
  
  // create the dialog box
  PROPSHEETHEADER psh;
  memset(&psh, 0, sizeof(psh));
  psh.dwSize      = sizeof(psh);
  psh.dwFlags     = PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE;
  psh.hwndParent  = NULL;
  psh.hInstance   = AfxGetApp()->m_hInstance;
  psh.hIcon       = NULL;
  psh.pszCaption  = "Sphere Options";
  psh.nPages      = num_pages;
  psh.nStartPage  = 0;
  psh.ppsp        = Pages;

  PropertySheet(&psh);
}

///////////////////////////////////////////////////////////

