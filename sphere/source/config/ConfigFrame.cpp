
#include "ConfigFrame.h"


/************************************************************************************/
// FRAME /////////////////////////////////////////////////////////////////////////////
/************************************************************************************/

CConfigFrame::CConfigFrame(const wxString& title)
            : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(400, 350),
                      wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN)
{

    wxPanel* panel      = new wxPanel(this,   wxID_ANY);

    wxBoxSizer* vbox1   = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* vbox2   = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* hbox1   = new wxBoxSizer(wxHORIZONTAL);

    m_restore_defaults  = new wxButton(panel, IDM_RESTORE_DEFAULTS, wxT("Restore Defaults"));
    m_ok_button         = new wxButton(panel, wxID_OK,     wxT("OK"));
    m_cancel_button     = new wxButton(panel, wxID_CANCEL, wxT("Cancel"));

    m_notebook          = new CConfigNotebook(this, panel, wxID_ANY);

    vbox2->Add(m_notebook, 1, wxEXPAND);

    hbox1->Add(m_restore_defaults);
    hbox1->Add(50, -1);
    hbox1->Add(m_ok_button);
    hbox1->Add(5, -1);
    hbox1->Add(m_cancel_button);

    vbox2->Add(-1, 10);
    vbox2->Add(hbox1, 0, wxALIGN_CENTER);
    vbox1->Add(vbox2, 1, wxEXPAND | wxALL, 10);

    panel->SetSizer(vbox1);

    Center();

    SetIcon(config_icon_xpm);

    Connect(IDM_RESTORE_DEFAULTS, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CConfigFrame::OnClickRestoreDefaults));
    Connect(wxID_OK,     wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CConfigFrame::OnClickOk));
    Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CConfigFrame::OnClickCancel));
}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigFrame::OnClickRestoreDefaults(wxCommandEvent& event)
{
    LoadDefaultConfiguration();

    m_notebook->GetVideoPage()  ->LoadConfiguration(true);
    m_notebook->GetAudioPage()  ->LoadConfiguration(true);
    m_notebook->GetInputPage()  ->LoadConfiguration(true);
    m_notebook->GetNetworkPage()->LoadConfiguration(true);
}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigFrame::OnClickOk(wxCommandEvent& event)
{
    if (!m_notebook->GetVideoPage()->IsDriverSelected())
    {
        wxString error(wxT("No video driver selected.\nPlease select one from the list."));

        wxMessageDialog dial(NULL, error, wxT("Sphere Configuration"), wxOK | wxICON_ERROR);
        dial.ShowModal();

        return;
    }

    SaveConfiguration();
    Close();
}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigFrame::OnClickCancel(wxCommandEvent& event)
{
    Close();
}

//////////////////////////////////////////////////////////////////////////////////////
bool
CConfigFrame::Initialize(wxString &error)
{

    if (!m_notebook->GetVideoPage()->BuildDriverList(error))
        return false;

    LoadConfiguration();

    m_notebook->GetVideoPage()  ->LoadConfiguration();
    m_notebook->GetAudioPage()  ->LoadConfiguration();
    m_notebook->GetInputPage()  ->LoadConfiguration();
    m_notebook->GetNetworkPage()->LoadConfiguration();

    return true;

}

//////////////////////////////////////////////////////////////////////////////////////
static void LoadDefaultPlayerConfiguration(PLAYERCONFIG &config, int player_index)
{

    switch (player_index)
    {
        case 0:
            config.up    = wxT("KEY_UP");
            config.down  = wxT("KEY_DOWN");
            config.left  = wxT("KEY_LEFT");
            config.right = wxT("KEY_RIGHT");
            config.allow_keyboard_input = true;
            config.allow_joypad_input   = true;
            break;

        case 1:
            config.up    = wxT("KEY_W");
            config.down  = wxT("KEY_S");
            config.left  = wxT("KEY_A");
            config.right = wxT("KEY_D");
            config.allow_keyboard_input = true;
            config.allow_joypad_input   = true;
            break;

        case 2:
            config.up    = wxT("KEY_I");
            config.down  = wxT("KEY_K");
            config.left  = wxT("KEY_J");
            config.right = wxT("KEY_L");
            config.allow_keyboard_input = true;
            config.allow_joypad_input   = true;
            break;

        case 3:
            config.up    = wxT("KEY_NUM_8");
            config.down  = wxT("KEY_NUM_5");
            config.left  = wxT("KEY_NUM_4");
            config.right = wxT("KEY_NUM_6");
            config.allow_keyboard_input = true;
            config.allow_joypad_input   = true;
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigFrame::LoadDefaultConfiguration()
{

    // load default audio configuration
    m_sphere_config.sound_preference = 0;
    m_sphere_config.audio_driver     = audio_drivers[0];

    // load default input configuration
    for (int i = 0; i < 4; ++i)
        LoadDefaultPlayerConfiguration(m_sphere_config.players[i], i);

    // load default network configuration
    m_sphere_config.allow_networking = true;

}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigFrame::LoadConfiguration()
{
    wxString filename(wxGetCwd() + wxT("/engine.ini"));

    if (!wxFile::Exists(filename))
    {
        LoadDefaultConfiguration();
        return;
    }

    wxFileInputStream is(filename);

    if (!is.IsOk())
    {
        LoadDefaultConfiguration();
        return;
    }

    wxFileConfig file(is);

    wxString sDummy;
    long     lDummy;
    bool     bDummy;

    // load video configuration
    file.Read(wxT("Video/Driver"), &sDummy, wxEmptyString);
    m_sphere_config.video_driver = sDummy;


    // load audio configuration
    file.Read(wxT("Audio/Preference"), &lDummy, (long)0);
    m_sphere_config.sound_preference = (int)lDummy;

    file.Read(wxT("Audio/Driver"), &sDummy, audio_drivers[0]);
    m_sphere_config.audio_driver = sDummy;


    // load input configuration
    for (int i = 0; i < 4; ++i)
    {
        file.Read(players[i] + wxT("/AllowKeyboardInput"), &bDummy, true);
        m_sphere_config.players[i].allow_keyboard_input = bDummy;

        file.Read(players[i] + wxT("/AllowJoypadInput"), &bDummy, true);
        m_sphere_config.players[i].allow_joypad_input = bDummy;

        file.Read(players[i] + wxT("/Up"), &sDummy, wxEmptyString);
        m_sphere_config.players[i].up = sDummy;

        file.Read(players[i] + wxT("/Down"), &sDummy, wxEmptyString);
        m_sphere_config.players[i].down = sDummy;

        file.Read(players[i] + wxT("/Left"), &sDummy, wxEmptyString);
        m_sphere_config.players[i].left = sDummy;

        file.Read(players[i] + wxT("/Right"), &sDummy, wxEmptyString);
        m_sphere_config.players[i].right = sDummy;
    }


    // load network configuration
    file.Read(wxT("Network/AllowNetworking"), &bDummy, true);
    m_sphere_config.allow_networking = bDummy;

}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigFrame::SaveConfiguration()
{

    wxFileConfig file(wxEmptyString, wxEmptyString, wxGetCwd() + wxT("/engine.ini"), wxEmptyString, wxCONFIG_USE_LOCAL_FILE);

    // save video configuration
    file.Write(wxT("Video/Driver"), m_sphere_config.video_driver);


    // save audio configuration
    file.Write(wxT("Audio/Preference"), m_sphere_config.sound_preference);
    file.Write(wxT("Audio/Driver"),     m_sphere_config.audio_driver);


    // save input configuration
    for (int i = 0; i < 4; ++i)
    {
        file.Write(players[i] + wxT("/AllowKeyboardInput"), m_sphere_config.players[i].allow_keyboard_input);
        file.Write(players[i] + wxT("/AllowJoypadInput"),   m_sphere_config.players[i].allow_joypad_input);
        file.Write(players[i] + wxT("/Up"),    m_sphere_config.players[i].up);
        file.Write(players[i] + wxT("/Down"),  m_sphere_config.players[i].down);
        file.Write(players[i] + wxT("/Left"),  m_sphere_config.players[i].left);
        file.Write(players[i] + wxT("/Right"), m_sphere_config.players[i].right);
    }


    // save network configuration
    file.Write(wxT("Network/AllowNetworking"), m_sphere_config.allow_networking);

}


/************************************************************************************/
// NOTEBOOK //////////////////////////////////////////////////////////////////////////
/************************************************************************************/

CConfigNotebook::CConfigNotebook(CConfigFrame* frame, wxWindow* parent, wxWindowID id)
               : wxNotebook(parent, id)
{
    m_frame   = frame;

    m_video   = new CConfigVideoPage(frame, this);
    m_audio   = new CConfigAudioPage(frame, this);
    m_input   = new CConfigInputPage(frame, this);
    m_network = new CConfigNetworkPage(frame, this);

    AddPage(m_video,   wxT("Video"));
    AddPage(m_audio,   wxT("Audio"));
    AddPage(m_input,   wxT("Input"));
    AddPage(m_network, wxT("Network"));


    Connect(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGING, wxNotebookEventHandler(CConfigNotebook::OnPageChanging));

}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigNotebook::OnPageChanging(wxNotebookEvent& event)
{
    if (m_input->GetManager()->IsRunning())
        event.Veto();
    else
        event.Allow();

}


/************************************************************************************/
// VIDEO PAGE ////////////////////////////////////////////////////////////////////////
/************************************************************************************/

CConfigVideoPage::CConfigVideoPage(CConfigFrame* frame, wxWindow* parent)
                : wxPanel(parent, wxID_ANY)
{
    m_frame = frame;

    wxStaticBox* box   = new wxStaticBox(this, wxID_ANY, wxT(" Video Settings"));

    m_driver_list      = new wxListBox(this,    IDV_DRIVER_LIST);
    m_config_button    = new wxButton(this,     IDV_CONFIG_BUTTON, wxT("Configure Driver"));
    m_info_name        = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);
    m_info_author      = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);
    m_info_date        = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);
    m_info_version     = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);
    m_info_desc        = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);

    wxStaticBoxSizer* sbox = new wxStaticBoxSizer(box, wxVERTICAL);
    wxBoxSizer* vbox       = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* vbox1      = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* hbox1      = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* hbox2      = new wxBoxSizer(wxHORIZONTAL);

    vbox1->Add(m_info_name,    1, wxEXPAND);
    vbox1->Add(m_info_author,  1, wxEXPAND);
    vbox1->Add(m_info_date,    1, wxEXPAND);
    vbox1->Add(m_info_version, 1, wxEXPAND);
    vbox1->Add(m_info_desc,    1, wxEXPAND);

    hbox1->Add(10, -1);
    hbox1->Add(m_driver_list, 1, wxEXPAND);
    hbox1->Add(10, -1);
    hbox1->Add(vbox1, 1, wxEXPAND);

    hbox2->Add(10, -1);
    hbox2->Add(m_config_button);

    sbox->Add(-1, 10);
    sbox->Add(hbox1, 1, wxEXPAND);
    sbox->Add(-1, 10);
    sbox->Add(hbox2, 0, wxBOTTOM, 10);

    vbox->Add(sbox, 1, wxEXPAND | wxALL, 10);

    SetSizer(vbox);

    Connect(IDV_CONFIG_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED,        wxCommandEventHandler(CConfigVideoPage::OnClickConfigureDriver));
    Connect(IDV_DRIVER_LIST,   wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler(CConfigVideoPage::OnClickConfigureDriver));
    Connect(IDV_DRIVER_LIST,   wxEVT_COMMAND_LISTBOX_SELECTED,      wxCommandEventHandler(CConfigVideoPage::OnSelectDriver));

}


/////////////////////////////////////////////////////////////////////////////////////////////////
void
CConfigVideoPage::OnClickConfigureDriver(wxCommandEvent& event)
{
    if (!IsDriverSelected())
    {
        wxString error(wxT("No video driver selected.\nPlease select one from the list."));

        wxMessageDialog dial(NULL, error, wxT("Sphere Configuration"), wxOK | wxICON_ERROR);
        dial.ShowModal();

        return;
    }

    wxFileName drv(m_driver_list->GetStringSelection());
    wxString   drv_name(drv.GetName());

    if (drv_name == wxT("standard32") ||
        drv_name == wxT("standard16") ||
        drv_name == wxT("sdl32"))
    {
        CDialogSphere32 dial(this, wxT("Configure ") + drv_name, drv_name);
        dial.ShowModal();

    }
    else if (drv_name == wxT("standard8"))
    {
        CDialogSphere8 dial(this, wxT("Configure ") + drv_name, drv_name);
        dial.ShowModal();
    }
    else if (drv_name == wxT("sphere_gl") ||
             drv_name == wxT("sdl_gl"))
    {
        CDialogSphereGL dial(this, wxT("Configure ") + drv_name, drv_name);
        dial.ShowModal();
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////////
void
CConfigVideoPage::OnSelectDriver(wxCommandEvent& event)
{
    wxString selected_drv(m_driver_list->GetString(event.GetInt()));

    m_frame->GetConfig()->video_driver = selected_drv;
    LoadDriverInfo(selected_drv);

    m_driver_list->Select(event.GetInt());
}

/////////////////////////////////////////////////////////////////////////////////////////////////
bool
CConfigVideoPage::IsDriverSelected()
{
    return m_driver_list->GetSelection() != wxNOT_FOUND;
}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigVideoPage::LoadConfiguration(bool reload)
{
    int video_index = m_driver_list->FindString(m_frame->GetConfig()->video_driver);

    if (video_index != wxNOT_FOUND)
    {
        m_driver_list->Select(video_index);
        wxString drv_name = m_driver_list->GetString(video_index);
        LoadDriverInfo(drv_name);
    }
}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigVideoPage::LoadDriverInfo(wxString &drv_name)
{
    DRIVERINFO drv_info;

    wxDynamicLibrary lib(wxGetCwd() + wxT("/system/video/") + drv_name, wxDL_LAZY);

    if (!lib.IsLoaded())
        return;

    if (!lib.HasSymbol(wxString(wxT("GetDriverInfo"))))
        return;

    void (STDCALL * get_driver_info)(DRIVERINFO* drv_info);

    get_driver_info = (void (STDCALL *)(DRIVERINFO*))lib.GetSymbol(wxString(wxT("GetDriverInfo")));

    get_driver_info(&drv_info);

    m_info_name   ->SetLabel(wxString(drv_info.name,        wxConvUTF8));
    m_info_author ->SetLabel(wxString(drv_info.author,      wxConvUTF8));
    m_info_date   ->SetLabel(wxString(drv_info.date,        wxConvUTF8));
    m_info_version->SetLabel(wxString(drv_info.version,     wxConvUTF8));
    m_info_desc   ->SetLabel(wxString(drv_info.description, wxConvUTF8));

}

//////////////////////////////////////////////////////////////////////////////////////
static bool IsValidDriver(wxString filename)
{
    wxDynamicLibrary lib(filename, wxDL_LAZY);

    if (!lib.IsLoaded())
        return false;

    if (!lib.HasSymbol(wxString(wxT("GetDriverInfo"))))
    {
        lib.Unload();
        return false;
    }

    lib.Unload();
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////
bool
CConfigVideoPage::BuildDriverList(wxString &error)
{
    // find, validate and load video drivers
    wxString video_dir(wxGetCwd() + wxT("/system/video/"));

    if (!wxDir::Exists(video_dir))
    {
        error = wxT("Could not enter <sphere>/system/video.\n"
                    "This probably means Sphere was installed incorrectly.\n"
                    "Sphere Configuration cannot continue.");
        return false;
    }

    wxDir dir(video_dir);

    wxArrayString driver_list;
    wxString      filename;

    bool succeeded = dir.GetFirst(&filename, wxT(FILESPEC_DYNLIB), wxDIR_FILES);

    if (succeeded && IsValidDriver(video_dir + filename))
        driver_list.Add(filename);

    while (succeeded)
    {
        succeeded = dir.GetNext(&filename);

        if (succeeded && IsValidDriver(video_dir + filename))
            driver_list.Add(filename);
    }

    if (driver_list.IsEmpty())
    {
        error = wxT("No video drivers found in <sphere>/system/video/.\n"
                    "This probably means Sphere was installed incorrectly.\n"
                    "Sphere Configuration cannot continue.");
        return false;
    }

    m_driver_list->InsertItems(driver_list, 0);

    return true;
}


/************************************************************************************/
// AUDIO PAGE ////////////////////////////////////////////////////////////////////////
/************************************************************************************/

CConfigAudioPage::CConfigAudioPage(CConfigFrame* frame, wxWindow* parent)
                : wxPanel(parent, wxID_ANY)
{
    m_frame = frame;

    wxStaticBox* box1 = new wxStaticBox(this, wxID_ANY, wxT(" Audio Settings"));
    wxStaticBox* box2 = new wxStaticBox(this, wxID_ANY, wxT("Driver"));

    m_sound_auto      = new wxRadioButton(this, IDA_SOUND_AUTO, wxT("Use sound if possible"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_sound_on        = new wxRadioButton(this, IDA_SOUND_ON,   wxT("Always use sound"));
    m_sound_off       = new wxRadioButton(this, IDA_SOUND_OFF,  wxT("No sound"));
    m_driver          = new wxChoice(this, IDA_DRIVER, wxDefaultPosition, wxSize(120, -1), MAX_AUDIO_DRIVERS, audio_drivers);

    wxStaticBoxSizer* sbox1 = new wxStaticBoxSizer(box1, wxHORIZONTAL);
    wxStaticBoxSizer* sbox2 = new wxStaticBoxSizer(box2, wxVERTICAL);
    wxBoxSizer* vbox        = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* vbox1       = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* vbox2       = new wxBoxSizer(wxVERTICAL);

    vbox1->Add(m_sound_auto, 0, wxALIGN_LEFT | wxLEFT | wxUP, 10);
    vbox1->Add(m_sound_on,   0, wxALIGN_LEFT | wxLEFT | wxUP, 10);
    vbox1->Add(m_sound_off,  0, wxALIGN_LEFT | wxLEFT | wxUP, 10);

    sbox2->Add(m_driver);

    vbox2->Add(sbox2, 0, wxALIGN_LEFT);

    sbox1->Add(vbox1);
    sbox1->Add(50, -1);
    sbox1->Add(vbox2);

    vbox->Add(sbox1, 1, wxEXPAND | wxALL, 10);

    SetSizer(vbox);


    Connect(IDA_SOUND_AUTO, wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(CConfigAudioPage::OnSelectSoundAuto));
    Connect(IDA_SOUND_ON,   wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(CConfigAudioPage::OnSelectSoundOn));
    Connect(IDA_SOUND_OFF,  wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(CConfigAudioPage::OnSelectSoundOff));
    Connect(IDA_DRIVER,     wxEVT_COMMAND_CHOICE_SELECTED,      wxCommandEventHandler(CConfigAudioPage::OnSelectDriver));

}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigAudioPage::OnSelectSoundAuto(wxCommandEvent& event)
{
    m_frame->GetConfig()->sound_preference = 0;
}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigAudioPage::OnSelectSoundOn(wxCommandEvent& event)
{
    m_frame->GetConfig()->sound_preference = 1;
}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigAudioPage::OnSelectSoundOff(wxCommandEvent& event)
{
    m_frame->GetConfig()->sound_preference = 2;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void
CConfigAudioPage::OnSelectDriver(wxCommandEvent& event)
{
    wxString selected_drv(m_driver->GetStringSelection());

    m_frame->GetConfig()->audio_driver = selected_drv;
}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigAudioPage::LoadConfiguration(bool reload)
{
    switch (m_frame->GetConfig()->sound_preference)
    {
        case 0:
            m_sound_auto->SetValue(true);
            break;
        case 1:
            m_sound_on->SetValue(true);
            break;
        case 2:
            m_sound_off->SetValue(true);
            break;
    }

    m_driver->SetStringSelection(m_frame->GetConfig()->audio_driver);
}


/************************************************************************************/
// INPUT PAGE ////////////////////////////////////////////////////////////////////////
/************************************************************************************/

CConfigInputPage::CConfigInputPage(CConfigFrame* frame, wxWindow* parent)
                : wxPanel(parent, wxID_ANY)
{
    m_frame = frame;

    m_input_manager        = new CInputManager(this);

    wxStaticBox* box       = new wxStaticBox(this, wxID_ANY, wxT(" Input Settings"));

    m_player_index         = new wxChoice(this,   IDI_PLAYER_INDEX, wxDefaultPosition, wxDefaultSize,  4, players);
    m_allow_keyboard_input = new wxCheckBox(this, IDI_ALLOW_KEYBOARD_INPUT, wxT("Allow Keyboard Input"));
    m_allow_joypad_input   = new wxCheckBox(this, IDI_ALLOW_JOYPAD_INPUT,   wxT("Allow Joypad Input"));

    m_up                   = new wxButton(this, IDI_UP,    wxT("Up"),    wxDefaultPosition, wxSize(50, 25));
    m_down                 = new wxButton(this, IDI_DOWN,  wxT("Down"),  wxDefaultPosition, wxSize(50, 25));
    m_left                 = new wxButton(this, IDI_LEFT,  wxT("Left"),  wxDefaultPosition, wxSize(50, 25));
    m_right                = new wxButton(this, IDI_RIGHT, wxT("Right"), wxDefaultPosition, wxSize(50, 25));

    m_up_str               = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_down_str             = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_left_str             = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_right_str            = new wxStaticText(this, wxID_ANY, wxEmptyString);

    wxStaticBoxSizer* sbox = new wxStaticBoxSizer(box, wxVERTICAL);
    wxBoxSizer* vbox       = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* hbox1      = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* hbox2      = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* hbox3      = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* hbox4      = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* hbox5      = new wxBoxSizer(wxHORIZONTAL);

    hbox1->Add(m_player_index);

    hbox2->Add(10, -1);
    hbox2->Add(m_up, 1);
    hbox2->Add(10, -1);
    hbox2->Add(m_up_str, 0, wxALIGN_CENTER_VERTICAL);

    hbox3->Add(10, -1);
    hbox3->Add(m_down, 1);
    hbox3->Add(10, -1);
    hbox3->Add(m_down_str, 0, wxALIGN_CENTER_VERTICAL);

    hbox4->Add(10, -1);
    hbox4->Add(m_left, 1);
    hbox4->Add(10, -1);
    hbox4->Add(m_left_str, 0, wxALIGN_CENTER_VERTICAL);

    hbox5->Add(10, -1);
    hbox5->Add(m_right, 1);
    hbox5->Add(10, -1);
    hbox5->Add(m_right_str, 0, wxALIGN_CENTER_VERTICAL);

    sbox->Add(-1, 5);
    sbox->Add(hbox1, 0, wxALIGN_CENTER_HORIZONTAL);
    sbox->Add(-1, 10);
    sbox->Add(hbox2);
    sbox->Add(hbox3);
    sbox->Add(hbox4);
    sbox->Add(hbox5);
    sbox->Add(-1, 15);
    sbox->Add(m_allow_keyboard_input, 0, wxALIGN_LEFT | wxLEFT, 10);
    sbox->Add(-1, 5);
    sbox->Add(m_allow_joypad_input,   0, wxALIGN_LEFT | wxLEFT, 10);

    vbox->Add(sbox, 1, wxEXPAND | wxALL, 10);

    SetSizer(vbox);

    Connect(IDI_PLAYER_INDEX,  wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(CConfigInputPage::OnSelectPlayer));
    Connect(IDI_UP,            wxEVT_COMMAND_BUTTON_CLICKED,  wxCommandEventHandler(CConfigInputPage::OnClickUp));
    Connect(IDI_DOWN,          wxEVT_COMMAND_BUTTON_CLICKED,  wxCommandEventHandler(CConfigInputPage::OnClickDown));
    Connect(IDI_LEFT,          wxEVT_COMMAND_BUTTON_CLICKED,  wxCommandEventHandler(CConfigInputPage::OnClickLeft));
    Connect(IDI_RIGHT,         wxEVT_COMMAND_BUTTON_CLICKED,  wxCommandEventHandler(CConfigInputPage::OnClickRight));
    Connect(IDI_ALLOW_KEYBOARD_INPUT, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(CConfigInputPage::OnClickAllowKeyboardInput));
    Connect(IDI_ALLOW_JOYPAD_INPUT,   wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(CConfigInputPage::OnClickAllowJoypadInput));

}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigInputPage::OnSelectPlayer(wxCommandEvent &event)
{
    LoadPlayerConfiguration(event.GetInt());
}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigInputPage::OnClickUp(wxCommandEvent& event)
{
    m_input_manager->Start(&m_frame->GetConfig()->players[GetCurrentPlayer()].up, m_up_str, 10);

}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigInputPage::OnClickDown(wxCommandEvent& event)
{
    m_input_manager->Start(&m_frame->GetConfig()->players[GetCurrentPlayer()].down, m_down_str, 10);

}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigInputPage::OnClickLeft(wxCommandEvent& event)
{
    m_input_manager->Start(&m_frame->GetConfig()->players[GetCurrentPlayer()].left, m_left_str, 10);

}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigInputPage::OnClickRight(wxCommandEvent& event)
{
    m_input_manager->Start(&m_frame->GetConfig()->players[GetCurrentPlayer()].right, m_right_str, 10);

}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigInputPage::OnClickAllowKeyboardInput(wxCommandEvent& event)
{
    m_frame->GetConfig()->players[GetCurrentPlayer()].allow_keyboard_input = event.IsChecked();
}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigInputPage::OnClickAllowJoypadInput(wxCommandEvent& event)
{
    m_frame->GetConfig()->players[GetCurrentPlayer()].allow_joypad_input = event.IsChecked();
}

//////////////////////////////////////////////////////////////////////////////////////
CInputManager*
CConfigInputPage::GetManager()
{
    return m_input_manager;
}

//////////////////////////////////////////////////////////////////////////////////////
int
CConfigInputPage::GetCurrentPlayer()
{
    return m_player_index->GetSelection();
}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigInputPage::LoadPlayerConfiguration(int player_index)
{
    m_up_str   ->SetLabel(m_frame->GetConfig()->players[player_index].up);
    m_down_str ->SetLabel(m_frame->GetConfig()->players[player_index].down);
    m_left_str ->SetLabel(m_frame->GetConfig()->players[player_index].left);
    m_right_str->SetLabel(m_frame->GetConfig()->players[player_index].right);

    m_allow_keyboard_input->SetValue(m_frame->GetConfig()->players[player_index].allow_keyboard_input);
    m_allow_joypad_input  ->SetValue(m_frame->GetConfig()->players[player_index].allow_joypad_input);
}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigInputPage::LoadConfiguration(bool reload)
{
    if (reload)
    {
        LoadPlayerConfiguration(GetCurrentPlayer());
        return;
    }

    m_player_index->Select(0);
    LoadPlayerConfiguration(0);

}


/************************************************************************************/
// NETWORK PAGE //////////////////////////////////////////////////////////////////////
/************************************************************************************/

CConfigNetworkPage::CConfigNetworkPage(CConfigFrame* frame, wxWindow* parent)
                  : wxPanel(parent, wxID_ANY)
{
    m_frame = frame;

    wxStaticBox*  box      = new wxStaticBox(this, wxID_ANY, wxT(" Network Settings"));

    m_allow_networking     = new wxCheckBox(this, IDN_ALLOW_NETWORKING, wxT("Allow Networking"));

    wxStaticBoxSizer* sbox = new wxStaticBoxSizer(box, wxVERTICAL);
    wxBoxSizer* vbox       = new wxBoxSizer(wxVERTICAL);

    sbox->Add(m_allow_networking, 0, wxALIGN_LEFT | wxLEFT | wxUP, 10);
    vbox->Add(sbox, 1, wxEXPAND | wxALL, 10);

    SetSizer(vbox);

    Connect(IDN_ALLOW_NETWORKING, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(CConfigNetworkPage::OnClickAllowNetworking));

}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigNetworkPage::OnClickAllowNetworking(wxCommandEvent& event)
{
    m_frame->GetConfig()->allow_networking = event.IsChecked();
}

//////////////////////////////////////////////////////////////////////////////////////
void
CConfigNetworkPage::LoadConfiguration(bool reload)
{
    m_allow_networking->SetValue(m_frame->GetConfig()->allow_networking);
}


/************************************************************************************/
// INPUT MANAGER /////////////////////////////////////////////////////////////////////
/************************************************************************************/

CInputManager::CInputManager(CConfigInputPage* parent) : wxWindow(parent, wxID_ANY),
                                                         m_timer(this, IDI_INPUT_MANAGER)
{
    m_parent  = parent;
    m_subject = 0;
    m_output  = 0;

    Connect(                   wxEVT_KILL_FOCUS, wxFocusEventHandler(CInputManager::OnKillFocus));
    Connect(                   wxEVT_KEY_UP,     wxKeyEventHandler(CInputManager::OnKeyUp));
    Connect(IDI_INPUT_MANAGER, wxEVT_TIMER,      wxTimerEventHandler(CInputManager::OnTimer));
}

//////////////////////////////////////////////////////////////////////////////////////
void
CInputManager::OnKillFocus(wxFocusEvent &event)
{
    if (IsRunning())
        SetFocus();
    else
        event.Skip();

}

//////////////////////////////////////////////////////////////////////////////////////
static bool wxKeyToSphereKey(int key, wxString &key_str)
{
    switch (key)
    {
        case 'A':                   key_str = wxT("KEY_A");             break;
        case 'B':                   key_str = wxT("KEY_B");             break;
        case 'C':                   key_str = wxT("KEY_C");             break;
        case 'D':                   key_str = wxT("KEY_D");             break;
        case 'E':                   key_str = wxT("KEY_E");             break;
        case 'F':                   key_str = wxT("KEY_F");             break;
        case 'G':                   key_str = wxT("KEY_G");             break;
        case 'H':                   key_str = wxT("KEY_H");             break;
        case 'I':                   key_str = wxT("KEY_I");             break;
        case 'J':                   key_str = wxT("KEY_J");             break;
        case 'K':                   key_str = wxT("KEY_K");             break;
        case 'L':                   key_str = wxT("KEY_L");             break;
        case 'M':                   key_str = wxT("KEY_M");             break;
        case 'N':                   key_str = wxT("KEY_N");             break;
        case 'O':                   key_str = wxT("KEY_O");             break;
        case 'P':                   key_str = wxT("KEY_P");             break;
        case 'Q':                   key_str = wxT("KEY_Q");             break;
        case 'R':                   key_str = wxT("KEY_R");             break;
        case 'S':                   key_str = wxT("KEY_S");             break;
        case 'T':                   key_str = wxT("KEY_T");             break;
        case 'U':                   key_str = wxT("KEY_U");             break;
        case 'V':                   key_str = wxT("KEY_V");             break;
        case 'W':                   key_str = wxT("KEY_W");             break;
        case 'X':                   key_str = wxT("KEY_X");             break;
        case 'Y':                   key_str = wxT("KEY_Y");             break;
        case 'Z':                   key_str = wxT("KEY_Z");             break;

        case '0':                   key_str = wxT("KEY_0");             break;
        case '1':                   key_str = wxT("KEY_1");             break;
        case '2':                   key_str = wxT("KEY_2");             break;
        case '3':                   key_str = wxT("KEY_3");             break;
        case '4':                   key_str = wxT("KEY_4");             break;
        case '5':                   key_str = wxT("KEY_5");             break;
        case '6':                   key_str = wxT("KEY_6");             break;
        case '7':                   key_str = wxT("KEY_7");             break;
        case '8':                   key_str = wxT("KEY_8");             break;
        case '9':                   key_str = wxT("KEY_9");             break;

        case ',':                   key_str = wxT("KEY_COMMA");         break;
        case '.':                   key_str = wxT("KEY_PERIOD");        break;
        case '-':                   key_str = wxT("KEY_MINUS");         break;
        case '[':                   key_str = wxT("KEY_OPENBRACE");     break;
        case ']':                   key_str = wxT("KEY_CLOSEBRACE");    break;
        case '+':                   key_str = wxT("KEY_EQUALS");        break;
        case '\\':                  key_str = wxT("KEY_BACKSLASH");     break;
        case '/':                   key_str = wxT("KEY_SLASH");         break;

        case WXK_F1:                key_str = wxT("KEY_F1");            break;
        case WXK_F2:                key_str = wxT("KEY_F2");            break;
        case WXK_F3:                key_str = wxT("KEY_F3");            break;
        case WXK_F4:                key_str = wxT("KEY_F4");            break;
        case WXK_F5:                key_str = wxT("KEY_F5");            break;
        case WXK_F6:                key_str = wxT("KEY_F6");            break;
        case WXK_F7:                key_str = wxT("KEY_F7");            break;
        case WXK_F8:                key_str = wxT("KEY_F8");            break;
        case WXK_F9:                key_str = wxT("KEY_F9");            break;
        case WXK_F10:               key_str = wxT("KEY_F10");           break;
        case WXK_F11:               key_str = wxT("KEY_F11");           break;
        case WXK_F12:               key_str = wxT("KEY_F12");           break;

        case WXK_SHIFT:             key_str = wxT("KEY_SHIFT");         break;
        case WXK_ALT:               key_str = wxT("KEY_ALT");           break;
        case WXK_CONTROL:           key_str = wxT("KEY_CTRL");          break;
        case WXK_BACK:              key_str = wxT("KEY_BACKSPACE");     break;
        case WXK_TAB:               key_str = wxT("KEY_TAB");           break;
        case WXK_RETURN:            key_str = wxT("KEY_ENTER");         break;
        case WXK_ESCAPE:            key_str = wxT("KEY_ESCAPE");        break;
        case WXK_SPACE:             key_str = wxT("KEY_SPACE");         break;
        case WXK_CAPITAL:           key_str = wxT("KEY_CAPSLOCK");      break;
        case WXK_SCROLL:            key_str = wxT("KEY_SCROLLOCK");     break;
        case WXK_NUMLOCK:           key_str = wxT("KEY_NUMLOCK");       break;

        case WXK_UP:                key_str = wxT("KEY_UP");            break;
        case WXK_DOWN:              key_str = wxT("KEY_DOWN");          break;
        case WXK_LEFT:              key_str = wxT("KEY_LEFT");          break;
        case WXK_RIGHT:             key_str = wxT("KEY_RIGHT");         break;

        case WXK_PAGEUP:            key_str = wxT("KEY_PAGEUP");        break;
        case WXK_PAGEDOWN:          key_str = wxT("KEY_PAGEDOWN");      break;
        case WXK_HOME:              key_str = wxT("KEY_HOME");          break;
        case WXK_END:               key_str = wxT("KEY_END");           break;
        case WXK_INSERT:            key_str = wxT("KEY_INSERT");        break;
        case WXK_DELETE:            key_str = wxT("KEY_DELETE");        break;

        case WXK_NUMPAD0:           key_str = wxT("KEY_NUM_0");         break;
        case WXK_NUMPAD1:           key_str = wxT("KEY_NUM_1");         break;
        case WXK_NUMPAD2:           key_str = wxT("KEY_NUM_2");         break;
        case WXK_NUMPAD3:           key_str = wxT("KEY_NUM_3");         break;
        case WXK_NUMPAD4:           key_str = wxT("KEY_NUM_4");         break;
        case WXK_NUMPAD5:           key_str = wxT("KEY_NUM_5");         break;
        case WXK_NUMPAD6:           key_str = wxT("KEY_NUM_6");         break;
        case WXK_NUMPAD7:           key_str = wxT("KEY_NUM_7");         break;
        case WXK_NUMPAD8:           key_str = wxT("KEY_NUM_8");         break;
        case WXK_NUMPAD9:           key_str = wxT("KEY_NUM_9");         break;

        case WXK_NUMPAD_UP:         key_str = wxT("KEY_UP");            break;
        case WXK_NUMPAD_DOWN:       key_str = wxT("KEY_DOWN");          break;
        case WXK_NUMPAD_LEFT:       key_str = wxT("KEY_LEFT");          break;
        case WXK_NUMPAD_RIGHT:      key_str = wxT("KEY_RIGHT");         break;

        case WXK_NUMPAD_PAGEUP:     key_str = wxT("KEY_PAGEUP");        break;
        case WXK_NUMPAD_PAGEDOWN:   key_str = wxT("KEY_PAGEDOWN");      break;
        case WXK_NUMPAD_HOME:       key_str = wxT("KEY_HOME");          break;
        case WXK_NUMPAD_END:        key_str = wxT("KEY_END");           break;
        case WXK_NUMPAD_INSERT:     key_str = wxT("KEY_INSERT");        break;
        case WXK_NUMPAD_DELETE:     key_str = wxT("KEY_DELETE");        break;

        case WXK_NUMPAD_ENTER:      key_str = wxT("KEY_ENTER");         break;

        default: return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////
void
CInputManager::OnKeyUp(wxKeyEvent &event)
{
    wxString key_str;

    if (wxKeyToSphereKey(event.GetKeyCode(), key_str))
        Finalize(key_str);

}

//////////////////////////////////////////////////////////////////////////////////////
void
CInputManager::OnTimer(wxTimerEvent &event)
{
    m_seconds--;

    UpdateOutput();

    if (m_seconds <= 0)
        Stop();

}

//////////////////////////////////////////////////////////////////////////////////////
void
CInputManager::UpdateOutput()
{
    if (m_output)
        m_output->SetLabel(wxString::Format(wxT("Waiting for keypress... (%dsec left)"), m_seconds));
}

//////////////////////////////////////////////////////////////////////////////////////
bool
CInputManager::IsRunning()
{
    return m_timer.IsRunning();
}

//////////////////////////////////////////////////////////////////////////////////////
void
CInputManager::Start(wxString* subject, wxStaticText* output, int num_seconds)
{
    if (!IsRunning())
    {
        SetFocus();

        m_seconds  = num_seconds;
        m_original = output->GetLabel();
        m_subject  = subject;
        m_output   = output;

        m_timer.Start(1000);

        UpdateOutput();
    }
}

//////////////////////////////////////////////////////////////////////////////////////
void
CInputManager::CleanUp()
{
    m_output  = NULL;
    m_subject = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////
void
CInputManager::Stop()
{
    if (IsRunning())
    {
        m_timer.Stop();

        if (m_output)
            m_output->SetLabel(m_original);

        CleanUp();
    }
}

//////////////////////////////////////////////////////////////////////////////////////
void
CInputManager::Finalize(wxString &new_str)
{
    if (IsRunning())
    {
        m_timer.Stop();

        if (m_output)
            m_output->SetLabel(new_str);

        if (m_subject)
            *m_subject = new_str;

        CleanUp();
    }
}



//////////////////////////////////////////////////////////////////////////////////////




