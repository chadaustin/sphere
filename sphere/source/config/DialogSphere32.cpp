
#include "DialogSphere32.h"


const wxString filters[] =
{
    wxT("Direct Scale"),
    wxT("Scale2x"),
    wxT("Eagle"),
    wxT("hq2x"),
    wxT("2xSaI"),
    wxT("Super 2xSaI"),
    wxT("Super Eagle"),
};


enum
{
    ID_DOUBLE_SCALE = 0,
    ID_FULLSCREEN,
    ID_VSYNC,
    ID_FILTERS,
};


CDialogSphere32::CDialogSphere32(wxWindow* parent, const wxString& title, const wxString &drv_name)
               : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(300, 190),
                          wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX)
{
    m_drv_name = drv_name;

    wxPanel* panel = new wxPanel(this, wxID_ANY);

    wxBoxSizer* vbox    = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* vbox1   = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* vbox2   = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* hbox1   = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* hbox2   = new wxBoxSizer(wxHORIZONTAL);

    m_ok_button     = new wxButton(this,   wxID_OK,         wxT("OK"));
    m_cancel_button = new wxButton(this,   wxID_CANCEL,     wxT("Cancel"));
    m_filters       = new wxRadioBox(this, ID_FILTERS,      wxT("Filter"), wxDefaultPosition, wxDefaultSize, 7, filters, 4, wxRA_SPECIFY_ROWS);
    m_double_scale  = new wxCheckBox(this, ID_DOUBLE_SCALE, wxT("Scale (2x)"));
    m_fullscreen    = new wxCheckBox(this, ID_FULLSCREEN,   wxT("Fullscreen"));
    m_vsync         = new wxCheckBox(this, ID_VSYNC,        wxT("VSync"));

    vbox1->Add(-1, 20);
    vbox1->Add(m_fullscreen);
    vbox1->Add(-1, 5);
    vbox1->Add(m_vsync);
    vbox1->Add(-1, 5);
    vbox1->Add(m_double_scale);

    hbox1->Add(5, -1);
    hbox1->Add(vbox1);
    hbox1->Add(20, -1);
    hbox1->Add(m_filters);

    hbox2->Add(m_ok_button);
    hbox2->Add(10, -1);
    hbox2->Add(m_cancel_button);

    vbox2->Add(hbox1, 1, wxEXPAND);
    vbox2->Add(-1, 10);
    vbox2->Add(hbox2, 0, wxALIGN_CENTER);

    vbox->Add(vbox2, 1, wxEXPAND | wxALL, 10);

    SetSizer(vbox);

    Center();

    Connect(wxID_OK,         wxEVT_COMMAND_BUTTON_CLICKED,   wxCommandEventHandler(CDialogSphere32::OnClickOk));
    Connect(wxID_CANCEL,     wxEVT_COMMAND_BUTTON_CLICKED,   wxCommandEventHandler(CDialogSphere32::OnClickCancel));
    Connect(ID_FULLSCREEN,   wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(CDialogSphere32::OnClickFullscreen));
    Connect(ID_DOUBLE_SCALE, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(CDialogSphere32::OnClickScale));

    LoadConfiguration();

}

//////////////////////////////////////////////////////////////////////////////////////
void
CDialogSphere32::OnClickOk(wxCommandEvent& event)
{
    SaveConfiguration();
    EndModal(1);
}

//////////////////////////////////////////////////////////////////////////////////////
void
CDialogSphere32::OnClickCancel(wxCommandEvent& event)
{
    EndModal(0);
}

//////////////////////////////////////////////////////////////////////////////////////
void
CDialogSphere32::OnClickFullscreen(wxCommandEvent& event)
{
    if (m_fullscreen->IsChecked())
        m_vsync->Enable();
    else
        m_vsync->Enable(false);
}

//////////////////////////////////////////////////////////////////////////////////////
void
CDialogSphere32::OnClickScale(wxCommandEvent& event)
{
    if (m_double_scale->IsChecked())
        m_filters->Enable();
    else
        m_filters->Enable(false);
}

//////////////////////////////////////////////////////////////////////////////////////
void
CDialogSphere32::LoadDefaultConfiguration()
{
    m_fullscreen->SetValue(false);
    m_vsync->SetValue(false);
    m_double_scale->SetValue(false);
    m_filters->SetSelection(0);

    m_vsync->Enable(false);
    m_filters->Enable(false);
}

//////////////////////////////////////////////////////////////////////////////////////
void
CDialogSphere32::LoadConfiguration()
{
    wxString filename(wxGetCwd() + wxT("/system/video/") + m_drv_name + wxT(".cfg"));

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

    long     lDummy;
    bool     bDummy;

    file.Read(m_drv_name + wxT("/Fullscreen"), &bDummy, false);
    m_fullscreen->SetValue(bDummy);

    file.Read(m_drv_name + wxT("/VSync"), &bDummy, false);
    m_vsync->SetValue(bDummy);

    file.Read(m_drv_name + wxT("/Scale"), &bDummy, false);
    m_double_scale->SetValue(bDummy);

    file.Read(m_drv_name + wxT("/Filter"), &lDummy, 0);
    if (lDummy > 6 || lDummy < 0) lDummy = 0;
    m_filters->SetSelection((int)lDummy);

    if (!m_fullscreen->IsChecked())
        m_vsync->Enable(false);

    if (!m_double_scale->IsChecked())
        m_filters->Enable(false);

}

//////////////////////////////////////////////////////////////////////////////////////
void
CDialogSphere32::SaveConfiguration()
{
    wxString filename(wxGetCwd() + wxT("/system/video/") + m_drv_name + wxT(".cfg"));

    wxFileConfig file(wxEmptyString, wxEmptyString, filename, wxEmptyString, wxCONFIG_USE_LOCAL_FILE);

    file.Write(m_drv_name + wxT("/Fullscreen"),  m_fullscreen->IsChecked());
    file.Write(m_drv_name + wxT("/VSync"),       m_vsync->IsChecked());
    file.Write(m_drv_name + wxT("/Scale"), m_double_scale->IsChecked());
    file.Write(m_drv_name + wxT("/Filter"),      (long)m_filters->GetSelection());

}








