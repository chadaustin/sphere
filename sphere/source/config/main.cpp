#pragma warning(disable : 4786)  // identifier too long

#include <windows.h>
#include <commctrl.h>
#include <prsht.h>
#include <vector>
#include <string>
#include <stdio.h>
#include "../engine/win32/win32_sphere_config.hpp"
#include "resource.h"
using namespace std;


const char* VIDEODRIVER_DIRECTORY = "system/video";

struct INTERNALDRIVERINFO
{
  string name;
  string author;
  string date;
  string version;
  string description;
};


struct DRIVERINFO
{
  const char* name;
  const char* author;
  const char* date;
  const char* version;
  const char* description;
};

const char* video_functions[] = {
    "GetDriverInfo",
    "ConfigureDriver",
    "InitVideoDriver",
    "CloseVideoDriver",
    "FlipScreen",
    "SetClippingRectangle",
    "GetClippingRectangle",
    "CreateImage",
    "GrabImage",
    "DestroyImage",
    "BlitImage",
    "BlitImageMask",
    "TransformBlitImage",
    "TransformBlitImageMask",
    "GetImageWidth",
    "GetImageHeight",
    "LockImage",
    "UnlockImage",
    "DirectBlit",
    "DirectTransformBlit",
    "DirectGrab",
    "DrawPoint",
    "DrawLine",
    "DrawGradientLine",
    "DrawTriangle",
    "DrawGradientTriangle",
    "DrawRectangle",
    "DrawGradientRectangle",
};

void BuildDriverList();
void LoadConfiguration();
void SaveConfiguration();
void ExecuteDialog();

bool IsValidDriver(const char* filename);

BOOL CALLBACK VideoDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
BOOL CALLBACK AudioDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
BOOL CALLBACK InputDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

bool ConfigureDriver(HWND window, const char* driver);
bool GetDriverInfo(const char* drivername, INTERNALDRIVERINFO* driverinfo);


HINSTANCE      MainInstance;
char           ConfigFile[MAX_PATH];
SPHERECONFIG   Config;
vector<string> VideoDriverList;


////////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
  MainInstance = instance;

  BuildDriverList();
  if (VideoDriverList.size() == 0)
  {
    MessageBox(
      NULL,
      "No video drivers found in <sphere>/system/video/.\n"
      "This probably means Sphere was installed incorrectly.\n"
      "Sphere Configuration cannot continue.",
      "Sphere Configuration",
      MB_OK);
    return 1;
  }

  LoadConfiguration();
  ExecuteDialog();
  SaveConfiguration();

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

void BuildDriverList()
{
  // enter system/video directory
  char old_directory[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, old_directory);
  SetCurrentDirectory(VIDEODRIVER_DIRECTORY);

  // enumerate DLLs
  WIN32_FIND_DATA ffd;
  HANDLE handle = FindFirstFile("*.dll", &ffd);
  if (handle != INVALID_HANDLE_VALUE)
  {
    do {

      // if it's a valid driver, add it to the list
      if (IsValidDriver(ffd.cFileName)) {
        VideoDriverList.push_back(ffd.cFileName);
      }

    } while (FindNextFile(handle, &ffd));

    FindClose(handle);
  }

  SetCurrentDirectory(old_directory);
}

////////////////////////////////////////////////////////////////////////////////

void LoadConfiguration()
{  
  // create the config filename
#ifdef NDEBUG
  GetModuleFileName(MainInstance, ConfigFile, MAX_PATH);
  if (*strrchr(ConfigFile, '\\'))
  {
    *strrchr(ConfigFile, '\\') = 0;
    strcat(ConfigFile, "\\engine.ini");
  }
  else
    strcpy(ConfigFile, "engine.ini");
#else
  GetCurrentDirectory(MAX_PATH, ConfigFile);
  strcat(ConfigFile, "\\engine.ini");
#endif

  // load the configuration
  LoadSphereConfig(&Config, ConfigFile);
}

////////////////////////////////////////////////////////////////////////////////

void SaveConfiguration()
{
  // must be called after LoadConfiguration()
  // save the configuration
  SaveSphereConfig(&Config, ConfigFile);
}

////////////////////////////////////////////////////////////////////////////////

void ExecuteDialog()
{
  PROPSHEETPAGE Pages[3];

  // default values
  for (int i = 0; i < sizeof(Pages) / sizeof(*Pages); i++)
  {
    Pages[i].dwSize    = sizeof(Pages[i]);
    Pages[i].dwFlags   = PSP_DEFAULT;
    Pages[i].hInstance = MainInstance;
    Pages[i].hIcon     = NULL;
  }
  
  // video page
  Pages[0].pszTemplate = MAKEINTRESOURCE(IDD_VIDEO_PAGE);
  Pages[0].pfnDlgProc  = VideoDialogProc;

  // audio page
  Pages[1].pszTemplate = MAKEINTRESOURCE(IDD_AUDIO_PAGE);
  Pages[1].pfnDlgProc  = AudioDialogProc;

  // input page
  Pages[2].pszTemplate = MAKEINTRESOURCE(IDD_INPUT_PAGE);
  Pages[2].pfnDlgProc  = InputDialogProc;

  // create the dialog box
  PROPSHEETHEADER psh;
  memset(&psh, 0, sizeof(psh));
  psh.dwSize      = sizeof(psh);
  psh.dwFlags     = PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE;
  psh.hwndParent  = NULL;
  psh.hInstance   = MainInstance;
  psh.hIcon       = NULL;
  psh.pszCaption  = "Sphere Configuration";
  psh.nPages      = sizeof(Pages) / sizeof(*Pages);
  psh.nStartPage  = 0;
  psh.ppsp        = Pages;

  PropertySheet(&psh);
}

////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK VideoDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {
    case WM_INITDIALOG:
    {
      // add the drivers
      for (int i = 0; i < VideoDriverList.size(); i++)
        SendDlgItemMessage(window, IDC_DRIVERLIST, LB_ADDSTRING, 0, (LPARAM)VideoDriverList[i].c_str());

      // select the driver in the list box
      SendDlgItemMessage(window, IDC_DRIVERLIST, LB_SELECTSTRING, 0, (LPARAM)Config.videodriver.c_str());

      // update the driver information
      SendMessage(window, WM_COMMAND, MAKEWPARAM(IDC_DRIVERLIST, LBN_SELCHANGE), 0);

      SetFocus(GetDlgItem(window, IDC_DRIVERLIST));
      return FALSE;
    }

    ////////////////////////////////////////////////////////////////////////////

    case WM_COMMAND:
      // driver configure
      if (LOWORD(wparam) == IDC_CONFIGURE_DRIVER)
      {
        char driver[MAX_PATH];
        int sel = SendDlgItemMessage(window, IDC_DRIVERLIST, LB_GETCURSEL, 0, 0);
        SendDlgItemMessage(window, IDC_DRIVERLIST, LB_GETTEXT, sel, (LPARAM)driver);
        ConfigureDriver(window, driver);
        return TRUE;
      }

      // new driver has been selected
      if (LOWORD(wparam) == IDC_DRIVERLIST && HIWORD(wparam) == LBN_SELCHANGE)
      {
        char drivername[MAX_PATH];
        int sel = SendDlgItemMessage(window, IDC_DRIVERLIST, LB_GETCURSEL, 0, 0);
        if (sel == -1)
          return TRUE;

        SendDlgItemMessage(window, IDC_DRIVERLIST, LB_GETTEXT, sel, (LPARAM)drivername);

        // retrieve the driver's information and put it in the controls
        INTERNALDRIVERINFO driverinfo;
        GetDriverInfo(drivername, &driverinfo);

        SetDlgItemText(window, IDC_NAME,        driverinfo.name.c_str());
        SetDlgItemText(window, IDC_AUTHOR,      driverinfo.author.c_str());
        SetDlgItemText(window, IDC_DATE,        driverinfo.date.c_str());
        SetDlgItemText(window, IDC_VERSION,     driverinfo.version.c_str());
        SetDlgItemText(window, IDC_DESCRIPTION, driverinfo.description.c_str());

        return TRUE;
      }

      return FALSE;

    ////////////////////////////////////////////////////////////////////////////

    case WM_NOTIFY:
    {
      PSHNOTIFY* psn = (PSHNOTIFY*)lparam;
      if (psn->hdr.code == PSN_APPLY)
      {
        // get the driver
        char buffer[512];
        int sel = SendDlgItemMessage(window, IDC_DRIVERLIST, LB_GETCURSEL, 0, 0);
        SendDlgItemMessage(window, IDC_DRIVERLIST, LB_GETTEXT, sel, (LPARAM)buffer);
        Config.videodriver = buffer;

        return TRUE;
      }

      return FALSE;
    }

    ////////////////////////////////////////////////////////////////////////////

    default:
      return FALSE;
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK AudioDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {
    case WM_INITDIALOG:
    {
      SetFocus(GetDlgItem(window, IDC_DRIVERLIST));

      switch (Config.sound) {
        case SOUND_ON:         CheckDlgButton(window, IDC_SOUND_ON,         BST_CHECKED); break;
        case SOUND_OFF:        CheckDlgButton(window, IDC_SOUND_OFF,        BST_CHECKED); break;
        case SOUND_AUTODETECT: 
        default:               CheckDlgButton(window, IDC_SOUND_AUTODETECT, BST_CHECKED); break;
      }

      return FALSE;
    }

    ////////////////////////////////////////////////////////////////////////////

    case WM_NOTIFY:
    {
      PSHNOTIFY* psn = (PSHNOTIFY*)lparam;
      if (psn->hdr.code == PSN_APPLY)
      {
        if (IsDlgButtonChecked(window, IDC_SOUND_ON) == BST_CHECKED) {
          Config.sound = SOUND_ON;
        } else if (IsDlgButtonChecked(window, IDC_SOUND_OFF) == BST_CHECKED) {
          Config.sound = SOUND_OFF;
        } else {
          Config.sound = SOUND_AUTODETECT;
        }
        return TRUE;
      }

      return FALSE;
    }

    ////////////////////////////////////////////////////////////////////////////

    default:
      return FALSE;
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK InputDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  window;
  message;
  wparam;
  lparam;

  switch (message)
  {
    case WM_COMMAND:
      return false;

    default:
      return FALSE;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool IsValidDriver(const char* filename)
{
  const char** functions = video_functions;
  int size = sizeof(video_functions) / sizeof(*video_functions);

  HINSTANCE module = LoadLibrary(filename);
  if (module == NULL) {
    return false;
  }

  for (int i = 0; i < size; i++) {
    if (GetProcAddress(module, functions[i]) == NULL) {
      FreeLibrary(module);
      return false;
    }
  }

  FreeLibrary(module);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool ConfigureDriver(HWND window, const char* driver)
{
  char old_directory[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, old_directory);
  SetCurrentDirectory(VIDEODRIVER_DIRECTORY);

  // load the driver
  HINSTANCE module = LoadLibrary(driver);
  if (driver == NULL)
  {
    SetCurrentDirectory(old_directory);
    return false;
  }

  // get the configuration function
  bool (__stdcall *configure_driver)(HWND parent) = (bool (_stdcall *)(HWND))
    GetProcAddress(module, "ConfigureDriver");
  if (configure_driver == NULL)
  {
    FreeLibrary(module);
    SetCurrentDirectory(old_directory);
    return false;
  }

  configure_driver(window);

  // clean up and go home
  FreeLibrary(module);
  SetCurrentDirectory(old_directory);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool GetDriverInfo(const char* drivername, INTERNALDRIVERINFO* driverinfo)
{
  // store old directory
  char old_directory[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, old_directory);
  SetCurrentDirectory(VIDEODRIVER_DIRECTORY);

  // load the driver
  HMODULE module = LoadLibrary(drivername);
  if (module == NULL)
  {
    SetCurrentDirectory(old_directory);
    return false;
  }

  // get the GetDriverInfo function
  void (__stdcall* get_driver_info)(DRIVERINFO* driverinfo) =
    (void (__stdcall*)(DRIVERINFO* driverinfo))GetProcAddress(module, "GetDriverInfo");

  if (get_driver_info == NULL)
  {
    FreeLibrary(module);
    SetCurrentDirectory(old_directory);
    return false;
  }

  // we must duplicate the information because once the driver is unloaded all of the memory is gone
  DRIVERINFO di;
  get_driver_info(&di);

  driverinfo->name        = di.name;
  driverinfo->author      = di.author;
  driverinfo->date        = di.date;
  driverinfo->version     = di.version;
  driverinfo->description = di.description;

  // clean up and go home
  FreeLibrary(module);
  SetCurrentDirectory(old_directory);
  return true;
}

////////////////////////////////////////////////////////////////////////////////
