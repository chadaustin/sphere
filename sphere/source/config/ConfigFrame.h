#ifndef CONFIG_FRAME_H
#define CONFIG_FRAME_H


#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/dir.h>
#include <wx/dynlib.h>
#include <wx/wfstream.h>
#include <wx/fileconf.h>
#include <wx/filename.h>
#include <wx/timer.h>

#include "resources/config.xpm"
#include "DialogSphereGL.h"
#include "DialogSphere32.h"
#include "DialogSphere8.h"


// platform dependent initialization
#if   defined(WIN32)
    #define FILESPEC_DYNLIB "*.dll"
    #define STDCALL __stdcall

    #define MAX_AUDIO_DRIVERS 2
    const wxString audio_drivers[] =
    {
        wxT("directsound"),
        wxT("winmm"),
    };

#elif defined(UNIX)
    #define FILESPEC_DYNLIB "*.so"
    #define STDCALL __attribute__((stdcall))

    #define MAX_AUDIO_DRIVERS 1
    const wxString audio_drivers[] =
    {
        wxT("oss"),
    };

#elif defined(MAC)
    #define FILESPEC_DYNLIB "*.dylib"
    #define STDCALL

    #define MAX_AUDIO_DRIVERS 1
    const wxString audio_drivers[] =
    {
        wxT("sdl_audio"),
    };

#else
    #error unknown platform

#endif


struct DRIVERINFO
{
    const char* name;
    const char* author;
    const char* date;
    const char* version;
    const char* description;
};


struct PLAYERCONFIG
{
    wxString up;
    wxString down;
    wxString left;
    wxString right;
    bool     allow_keyboard_input;
    bool     allow_joypad_input;
};


struct SPHERECONFIG
{
    // video
    wxString video_driver;

    // audio
    int sound_preference;
    wxString audio_driver;

    // input
    PLAYERCONFIG players[4];

    // network
    bool allow_networking;

};


const wxString players[] =
{
    wxT("Player1"),
    wxT("Player2"),
    wxT("Player3"),
    wxT("Player4"),
};


enum
{
    IDM_RESTORE_DEFAULTS = 0,

    IDV_CONFIG_BUTTON,
    IDV_DRIVER_LIST,

    IDA_SOUND_AUTO,
    IDA_SOUND_ON,
    IDA_SOUND_OFF,
    IDA_DRIVER,

    IDI_PLAYER_INDEX,
    IDI_UP,
    IDI_DOWN,
    IDI_LEFT,
    IDI_RIGHT,
    IDI_ALLOW_KEYBOARD_INPUT,
    IDI_ALLOW_JOYPAD_INPUT,
    IDI_INPUT_MANAGER,

    IDN_ALLOW_NETWORKING,
};


// forward declarations
class CConfigNotebook;
class CConfigVideoPage;
class CConfigAudioPage;
class CConfigInputPage;
class CConfigNetworkPage;
class CInputManager;


/************************************************************************************/
// FRAME /////////////////////////////////////////////////////////////////////////////
/************************************************************************************/

class CConfigFrame : public wxFrame
{
    public:

        CConfigFrame(const wxString& title);

        bool Initialize(wxString &error);

        void OnClickRestoreDefaults(wxCommandEvent& event);
        void OnClickOk(wxCommandEvent& event);
        void OnClickCancel(wxCommandEvent& event);

        void LoadDefaultConfiguration();
        void LoadConfiguration();
        void SaveConfiguration();

        SPHERECONFIG* GetConfig()
        {
            return &m_sphere_config;
        }

    private:

        SPHERECONFIG        m_sphere_config;

        wxButton*           m_restore_defaults;
        wxButton*           m_ok_button;
        wxButton*           m_cancel_button;

        CConfigNotebook*    m_notebook;


};


/************************************************************************************/
// NOTEBOOK //////////////////////////////////////////////////////////////////////////
/************************************************************************************/

class CConfigNotebook : public wxNotebook
{

    public:

        CConfigNotebook(CConfigFrame* frame, wxWindow* parent, wxWindowID id);

        void OnPageChanging(wxNotebookEvent& event);

        CConfigVideoPage*   GetVideoPage()
        {
            return m_video;
        }

        CConfigAudioPage*   GetAudioPage()
        {
            return m_audio;
        }

        CConfigInputPage*   GetInputPage()
        {
            return m_input;
        }

        CConfigNetworkPage* GetNetworkPage()
        {
            return m_network;
        }

    private:

        CConfigFrame*       m_frame;

        CConfigVideoPage*   m_video;
        CConfigAudioPage*   m_audio;
        CConfigInputPage*   m_input;
        CConfigNetworkPage* m_network;

};


/************************************************************************************/
// VIDEO PAGE ////////////////////////////////////////////////////////////////////////
/************************************************************************************/

class CConfigVideoPage : public wxPanel
{

    public:

        CConfigVideoPage(CConfigFrame* frame, wxWindow* parent);

        void OnClickConfigureDriver(wxCommandEvent& event);
        void OnSelectDriver(wxCommandEvent& event);

        bool IsDriverSelected();
        void LoadConfiguration(bool reload = false);
        bool BuildDriverList(wxString &error);
        void LoadDriverInfo(wxString &drv_name);

        CConfigFrame*  m_frame;

        wxListBox*     m_driver_list;
        wxButton*      m_config_button;
        wxStaticText*  m_info_name;
        wxStaticText*  m_info_author;
        wxStaticText*  m_info_date;
        wxStaticText*  m_info_version;
        wxStaticText*  m_info_desc;

};


/************************************************************************************/
// AUDIO PAGE ////////////////////////////////////////////////////////////////////////
/************************************************************************************/

class CConfigAudioPage : public wxPanel
{

    public:

        CConfigAudioPage(CConfigFrame* frame, wxWindow* parent);

        void OnSelectSoundAuto(wxCommandEvent& event);
        void OnSelectSoundOn(wxCommandEvent& event);
        void OnSelectSoundOff(wxCommandEvent& event);
        void OnSelectDriver(wxCommandEvent& event);

        void LoadConfiguration(bool reload = false);

        CConfigFrame*  m_frame;

        wxRadioButton* m_sound_auto;
        wxRadioButton* m_sound_on;
        wxRadioButton* m_sound_off;
        wxChoice*      m_driver;

};


/************************************************************************************/
// INPUT PAGE ////////////////////////////////////////////////////////////////////////
/************************************************************************************/

class CConfigInputPage : public wxPanel
{

    public:

        CConfigInputPage(CConfigFrame* frame, wxWindow* parent);

        void OnSelectPlayer(wxCommandEvent& event);
        void OnClickUp(wxCommandEvent& event);
        void OnClickDown(wxCommandEvent& event);
        void OnClickLeft(wxCommandEvent& event);
        void OnClickRight(wxCommandEvent& event);
        void OnClickAllowKeyboardInput(wxCommandEvent& event);
        void OnClickAllowJoypadInput(wxCommandEvent& event);

        CInputManager* GetManager();
        int  GetCurrentPlayer();
        void LoadPlayerConfiguration(int player_index);
        void LoadConfiguration(bool reload = false);

        CConfigFrame*  m_frame;

        CInputManager* m_input_manager;

        wxChoice*      m_player_index;
        wxButton*      m_up;
        wxButton*      m_down;
        wxButton*      m_left;
        wxButton*      m_right;
        wxStaticText*  m_up_str;
        wxStaticText*  m_down_str;
        wxStaticText*  m_left_str;
        wxStaticText*  m_right_str;
        wxCheckBox*    m_allow_keyboard_input;
        wxCheckBox*    m_allow_joypad_input;

};


/************************************************************************************/
// NETWORK PAGE //////////////////////////////////////////////////////////////////////
/************************************************************************************/

class CConfigNetworkPage : public wxPanel
{

    public:

        CConfigNetworkPage(CConfigFrame* frame, wxWindow* parent);

        void OnClickAllowNetworking(wxCommandEvent& event);

        void LoadConfiguration(bool reload = false);

        CConfigFrame*  m_frame;

        wxCheckBox*    m_allow_networking;

};


/************************************************************************************/
// INPUT MANAGER /////////////////////////////////////////////////////////////////////
/************************************************************************************/

class CInputManager : public wxWindow
{

    public:

        CInputManager(CConfigInputPage* parent);

        bool IsRunning();
        void Start(wxString* subject, wxStaticText* output, int num_seconds);
        void Stop();
        void Finalize(wxString &new_str);

        void OnKeyUp(wxKeyEvent &event);
        void OnKillFocus(wxFocusEvent &event);
        void OnTimer(wxTimerEvent &event);

    private:

        void CleanUp();
        void UpdateOutput();

        CConfigInputPage* m_parent;

        wxTimer       m_timer;
        int           m_seconds;

        wxString*     m_subject;
        wxStaticText* m_output;
        wxString      m_original;
};




#endif


