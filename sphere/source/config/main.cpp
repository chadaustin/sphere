#ifdef _MSC_VER
#pragma warning(disable : 4786)  // identifier too long
#endif

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
BOOL CALLBACK NetworkDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

bool ConfigureDriver(HWND window, const char* driver);
bool GetDriverInfo(const char* drivername, INTERNALDRIVERINFO* driverinfo);


HINSTANCE      MainInstance;
char           ConfigFile[MAX_PATH];
SPHERECONFIG   Config;
vector<string> VideoDriverList;


////////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
  // get base Sphere path
  char sphere_path[MAX_PATH];
  GetModuleFileName(instance, sphere_path, MAX_PATH);
  char* last_backslash = strrchr(sphere_path, '\\');
  if (last_backslash) {
    *last_backslash = 0;
  }
  SetCurrentDirectory(sphere_path);


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

#if 1
  struct Local
  {
    static void ValidateDriver(vector<string>& video_driver_list,
                               const char* folder_name, const char* build_type)
    {
      std::string filename = folder_name;
      filename += "/";
      filename += build_type;
      filename += "/";
      filename += folder_name;
      filename += ".dll";

      FILE* f = fopen(filename.c_str(), "rb");
      if (!f) return;
      fclose(f);

      if (IsValidDriver(filename.c_str())) {
        filename = "../../../win/desktop/sphere/source/video/" + filename;
        video_driver_list.push_back(filename);
      }
    }
  };
  
  if (SetCurrentDirectory("C:\\win\\Desktop\\sphere\\source\\video") == 0)
    return;

  handle = FindFirstFile("*", &ffd);
  if (handle != INVALID_HANDLE_VALUE)
  {
    do {
      if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        && ffd.cFileName != "." && ffd.cFileName != ".."
        && ffd.cFileName != "CVS") {
        Local::ValidateDriver(VideoDriverList, ffd.cFileName, "debug");
        Local::ValidateDriver(VideoDriverList, ffd.cFileName, "profile");
        Local::ValidateDriver(VideoDriverList, ffd.cFileName, "release");
      }
    } while (FindNextFile(handle, &ffd));

    FindClose(handle);
  }

  SetCurrentDirectory(old_directory);
#endif
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
  PROPSHEETPAGE Pages[4];

  // default values
  for (unsigned i = 0; i < sizeof(Pages) / sizeof(*Pages); i++)
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

  // network page
  Pages[3].pszTemplate = MAKEINTRESOURCE(IDD_NETWORK_PAGE);
  Pages[3].pfnDlgProc  = NetworkDialogProc;
  
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
      for (unsigned i = 0; i < VideoDriverList.size(); i++)
        SendDlgItemMessage(window, IDC_DRIVERLIST, LB_ADDSTRING, 0, (LPARAM)VideoDriverList[i].c_str());

      // select the driver in the list box
      SendDlgItemMessage(window, IDC_DRIVERLIST, LB_SELECTSTRING, 0, (LPARAM)Config.videodriver.c_str());

      // update the driver information
      SendMessage(window, WM_COMMAND, MAKEWPARAM(IDC_DRIVERLIST, LBN_SELCHANGE), 0);

      if ((int) SendDlgItemMessage(window, IDC_DRIVERLIST, LB_GETCURSEL, 0, 0) == -1) {
        EnableWindow(GetDlgItem(window, IDC_CONFIGURE_DRIVER), FALSE);
      }

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
        if (sel != -1) { 
          SendDlgItemMessage(window, IDC_DRIVERLIST, LB_GETTEXT, sel, (LPARAM)driver);
          ConfigureDriver(window, driver);
        }
        return TRUE;
      }

      // new driver has been selected
      if (LOWORD(wparam) == IDC_DRIVERLIST && HIWORD(wparam) == LBN_SELCHANGE)
      {
        char drivername[MAX_PATH];
        int sel = SendDlgItemMessage(window, IDC_DRIVERLIST, LB_GETCURSEL, 0, 0);
        if (sel == -1) {
          return TRUE;
        }
        else {
          EnableWindow(GetDlgItem(window, IDC_CONFIGURE_DRIVER), TRUE);
        }

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
      if (psn->hdr.code == UINT(PSN_APPLY))
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
      // add the drivers
      SendDlgItemMessage(window, IDC_SOUND_DRIVER, CB_ADDSTRING, 0, (LPARAM)"directsound");
      SendDlgItemMessage(window, IDC_SOUND_DRIVER, CB_ADDSTRING, 0, (LPARAM)"winmm");

      SendDlgItemMessage(window, IDC_SOUND_DRIVER, CB_SETCURSEL, 0, 0);

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
      if (psn->hdr.code == UINT(PSN_APPLY))
      {
        if (IsDlgButtonChecked(window, IDC_SOUND_ON) == BST_CHECKED) {
          Config.sound = SOUND_ON;
        } else if (IsDlgButtonChecked(window, IDC_SOUND_OFF) == BST_CHECKED) {
          Config.sound = SOUND_OFF;
        } else {
          Config.sound = SOUND_AUTODETECT;
        }

        if (SendDlgItemMessage(window, IDC_SOUND_DRIVER,    CB_GETCURSEL, 0, 0) == 0) {
          Config.audiodriver = "directsound";
        }
        if (SendDlgItemMessage(window, IDC_SOUND_DRIVER,    CB_GETCURSEL, 0, 0) == 1) {
          Config.audiodriver = "winmm";
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

BOOL CALLBACK NetworkDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {
    case WM_INITDIALOG:
    {
      if (Config.allow_networking) {
        CheckDlgButton(window, IDC_ALLOW_NETWORKING, BST_CHECKED);
      }

      return FALSE;
    }

    ////////////////////////////////////////////////////////////////////////////

    case WM_NOTIFY:
    {
      PSHNOTIFY* psn = (PSHNOTIFY*)lparam;
      if (psn->hdr.code == UINT(PSN_APPLY))
      {
        if (IsDlgButtonChecked(window, IDC_ALLOW_NETWORKING) == BST_CHECKED) {
          Config.allow_networking = true;
        } else {
          Config.allow_networking = false;
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
  static int current_player = 0;

  struct Local {
  
    static const char* GetUpKey(struct SPHERECONFIG* config, int player)
    {
      if (player >= 0 && player < 4) {
        return config->player_configurations[player].key_up_str;
      }

      return "";
    }

    static const char* GetDownKey(struct SPHERECONFIG* config, int player)
    {
      if (player >= 0 && player < 4) {
        return config->player_configurations[player].key_down_str;
      }

      return "";
    }

    static const char* GetLeftKey(struct SPHERECONFIG* config, int player)
    {
      if (player >= 0 && player < 4) {
        return config->player_configurations[player].key_left_str;
      }

      return "";
    }

    static const char* GetRightKey(struct SPHERECONFIG* config, int player)
    {
      if (player >= 0 && player < 4) {
        return config->player_configurations[player].key_right_str;
      }

      return "";
    }


    static bool IsKeyboardInputAllowed(struct SPHERECONFIG* config, int player)
    {
      if (player >= 0 && player < 4) {
        return config->player_configurations[player].keyboard_input_allowed;
      }

      return false;
    }

    static bool IsJoypadInputAllowed(struct SPHERECONFIG* config, int player)
    {
      if (player >= 0 && player < 4) {
        return config->player_configurations[player].joypad_input_allowed;
      }

      return false;
    } 
  };

      const char* keys[] = {
        " ",
        "KEY_UP",
        "KEY_DOWN",
        "KEY_RIGHT",
        "KEY_LEFT",
        "KEY_TILDE",
        "KEY_0",
        "KEY_1",
        "KEY_2",
        "KEY_3",
        "KEY_4",
        "KEY_5",
        "KEY_6",
        "KEY_7",
        "KEY_8",
        "KEY_9",
        "KEY_MINUS",
        "KEY_EQUALS",
        "KEY_A",
        "KEY_B",
        "KEY_C",
        "KEY_D",
        "KEY_E",
        "KEY_F",
        "KEY_G",
        "KEY_H",
        "KEY_I",
        "KEY_J",
        "KEY_K",
        "KEY_L",
        "KEY_M",
        "KEY_N",
        "KEY_O",
        "KEY_P",
        "KEY_Q",
        "KEY_R",
        "KEY_S",
        "KEY_T",
        "KEY_U",
        "KEY_V",
        "KEY_W",
        "KEY_X",
        "KEY_Y",
        "KEY_Z",
        "KEY_SPACE",
        "KEY_OPENBRACE",
        "KEY_CLOSEBRACE",
        "KEY_SEMICOLON",
        "KEY_APOSTROPHE",
        "KEY_COMMA",
        "KEY_PERIOD",
        "KEY_SLASH",
        "KEY_BACKSLASH",
        "KEY_NUM_0",
        "KEY_NUM_1",
        "KEY_NUM_2",
        "KEY_NUM_3",
        "KEY_NUM_4",
        "KEY_NUM_5",
        "KEY_NUM_6",
        "KEY_NUM_7",
        "KEY_NUM_8",
        "KEY_NUM_9"
      };

  switch (message)
  {
    case WM_INITDIALOG:
    {
      // add the players
      for (unsigned int i = 0; i < 4; i++) {
        char player_index[100] = {0};
        sprintf (player_index, "%d", (int) i);
        SendDlgItemMessage(window, IDC_PLAYER_INDEX, CB_ADDSTRING, 0, (LPARAM)player_index);       
      }

      SendDlgItemMessage(window, IDC_PLAYER_INDEX, CB_SETCURSEL, 0, 0);

      for (unsigned int i = 0; i < sizeof(keys) / sizeof(*keys); i++) {
        SendDlgItemMessage(window, IDC_KEYCOMBO_UP,    CB_ADDSTRING, 0, (LPARAM)keys[i]);
        SendDlgItemMessage(window, IDC_KEYCOMBO_DOWN,  CB_ADDSTRING, 0, (LPARAM)keys[i]);
        SendDlgItemMessage(window, IDC_KEYCOMBO_LEFT,  CB_ADDSTRING, 0, (LPARAM)keys[i]);
        SendDlgItemMessage(window, IDC_KEYCOMBO_RIGHT, CB_ADDSTRING, 0, (LPARAM)keys[i]);
      } 

      if (1) {
        int player = 0;
        SendDlgItemMessage(window, IDC_KEYCOMBO_UP,    CB_SELECTSTRING, 0, (LPARAM)Local::GetUpKey(&Config, player));
        SendDlgItemMessage(window, IDC_KEYCOMBO_DOWN,  CB_SELECTSTRING, 0, (LPARAM)Local::GetDownKey(&Config, player));
        SendDlgItemMessage(window, IDC_KEYCOMBO_LEFT,  CB_SELECTSTRING, 0, (LPARAM)Local::GetLeftKey(&Config, player));
        SendDlgItemMessage(window, IDC_KEYCOMBO_RIGHT, CB_SELECTSTRING, 0, (LPARAM)Local::GetRightKey(&Config, player));
        CheckDlgButton(window, IDC_KEYBOARD_INPUT, (Config.player_configurations[player].keyboard_input_allowed ? BST_CHECKED : BST_UNCHECKED));
        CheckDlgButton(window, IDC_JOYPAD_INPUT,   (Config.player_configurations[player].joypad_input_allowed   ? BST_CHECKED : BST_UNCHECKED));
      }

      SetFocus(GetDlgItem(window, IDC_PLAYER_INDEX));
      return FALSE;
    }

    case WM_NOTIFY:
    {
      PSHNOTIFY* psn = (PSHNOTIFY*)lparam;
      if (psn->hdr.code == UINT(PSN_APPLY))
      {
        strcpy(Config.player_configurations[current_player].key_up_str,     keys[SendDlgItemMessage(window, IDC_KEYCOMBO_UP,    CB_GETCURSEL, 0, 0)]);
        strcpy(Config.player_configurations[current_player].key_down_str,   keys[SendDlgItemMessage(window, IDC_KEYCOMBO_DOWN,  CB_GETCURSEL, 0, 0)]);
        strcpy(Config.player_configurations[current_player].key_left_str,   keys[SendDlgItemMessage(window, IDC_KEYCOMBO_LEFT,  CB_GETCURSEL, 0, 0)]);
        strcpy(Config.player_configurations[current_player].key_right_str,  keys[SendDlgItemMessage(window, IDC_KEYCOMBO_RIGHT, CB_GETCURSEL, 0, 0)]);
        Config.player_configurations[current_player].keyboard_input_allowed = IsDlgButtonChecked(window, IDC_KEYBOARD_INPUT) ? true : false;
        Config.player_configurations[current_player].joypad_input_allowed   = IsDlgButtonChecked(window, IDC_JOYPAD_INPUT)   ? true : false;
        return TRUE;
      }

      return FALSE;
    }
    break;

    case WM_COMMAND:
    {
     if (LOWORD(wparam) == IDC_PLAYER_INDEX && HIWORD(wparam) == CBN_SELCHANGE)
     {
       strcpy(Config.player_configurations[current_player].key_up_str,     keys[SendDlgItemMessage(window, IDC_KEYCOMBO_UP,    CB_GETCURSEL, 0, 0)]);
       strcpy(Config.player_configurations[current_player].key_down_str,   keys[SendDlgItemMessage(window, IDC_KEYCOMBO_DOWN,  CB_GETCURSEL, 0, 0)]);
       strcpy(Config.player_configurations[current_player].key_left_str,   keys[SendDlgItemMessage(window, IDC_KEYCOMBO_LEFT,  CB_GETCURSEL, 0, 0)]);
       strcpy(Config.player_configurations[current_player].key_right_str,  keys[SendDlgItemMessage(window, IDC_KEYCOMBO_RIGHT, CB_GETCURSEL, 0, 0)]);
       Config.player_configurations[current_player].keyboard_input_allowed = IsDlgButtonChecked(window, IDC_KEYBOARD_INPUT) ? true : false;
       Config.player_configurations[current_player].joypad_input_allowed   = IsDlgButtonChecked(window, IDC_JOYPAD_INPUT)   ? true : false;

       int player = SendDlgItemMessage(window, IDC_PLAYER_INDEX, CB_GETCURSEL, 0, 0);
       SendDlgItemMessage(window, IDC_KEYCOMBO_UP,    CB_SELECTSTRING, 0, (LPARAM)Local::GetUpKey(&Config, player));
       SendDlgItemMessage(window, IDC_KEYCOMBO_DOWN,  CB_SELECTSTRING, 0, (LPARAM)Local::GetDownKey(&Config, player));
       SendDlgItemMessage(window, IDC_KEYCOMBO_LEFT,  CB_SELECTSTRING, 0, (LPARAM)Local::GetLeftKey(&Config, player));
       SendDlgItemMessage(window, IDC_KEYCOMBO_RIGHT, CB_SELECTSTRING, 0, (LPARAM)Local::GetRightKey(&Config, player));
       CheckDlgButton(window, IDC_KEYBOARD_INPUT, (Config.player_configurations[player].keyboard_input_allowed ? BST_CHECKED : BST_UNCHECKED));
       CheckDlgButton(window, IDC_JOYPAD_INPUT,   (Config.player_configurations[player].joypad_input_allowed   ? BST_CHECKED : BST_UNCHECKED));
       current_player = player;
     }

     return FALSE;
    }
    break;

  }

  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

static FILE* s_Log = NULL;

void WriteLog(const char* text) {

  static bool firstcall = true;
  if (firstcall) {
    s_Log = fopen("drivers.log", "wb+");
    firstcall = false;

    if (s_Log) {
      fprintf(s_Log, "Driver Log File\n");
    }
  }
  else {
    s_Log = fopen("drivers.log", "a");
  }

  if (s_Log) {
    fwrite(text, sizeof(char), strlen(text), s_Log);
  }

  if (s_Log) {
    fclose(s_Log);
    s_Log = NULL;
  }
}

void WriteError(DWORD error_code)
{
  LPVOID lpMsgBuf;
  if (!FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    error_code,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPTSTR) &lpMsgBuf,
    0,
    NULL ))
  {
    return;
  }

  char string[1024] = {0};
  sprintf(string, "Error: %d - %s\n", error_code, (LPCTSTR)lpMsgBuf);
  WriteLog(string);

  LocalFree(lpMsgBuf);
}

bool IsValidDriver(const char* filename)
{
  const char** functions = video_functions;
  int size = sizeof(video_functions) / sizeof(*video_functions);

  HINSTANCE module = LoadLibrary(filename);
  if (module == NULL) {

    char string[MAX_PATH + 256] = {0};
    sprintf (string, "Cannot load library: '%s'\n", filename);
    WriteLog(string);
    WriteError(GetLastError());

    return false;
  }

  for (int i = 0; i < size; i++) {
    if (GetProcAddress(module, functions[i]) == NULL) {
      DWORD error_code = GetLastError();
      FreeLibrary(module);

      char string[MAX_PATH + 256] = {0};
      sprintf (string, "Cannot load function address: '%s' '%s'", functions[i], filename);
      WriteLog(string);
      WriteError(error_code);

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
