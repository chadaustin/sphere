
#include "DialogSphereGL.h"


const wxString bit_depths[] =
{
    wxT("16-bit"),
    wxT("24-bit"),
    wxT("32-bit"),
};


enum
{
    ID_DOUBLE_SCALE = 0,
    ID_BILINEAR_FILTER,
    ID_FULLSCREEN,
    ID_VSYNC,
    ID_BIT_DEPTHS,
};


CDialogSphereGL::CDialogSphereGL(wxWindow* parent, const wxString &title, const wxString &drv_name)
               : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(200, 180),
                          wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX)
{
    m_drv_name = drv_name;

    wxPanel* panel = new wxPanel(this, wxID_ANY);

    wxBoxSizer* vbox    = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* vbox1   = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* vbox2   = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* hbox1   = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* hbox2   = new wxBoxSizer(wxHORIZONTAL);

    m_ok_button       = new wxButton(this,   wxID_OK,            wxT("OK"));
    m_cancel_button   = new wxButton(this,   wxID_CANCEL,        wxT("Cancel"));
    m_bit_depths      = new wxRadioBox(this, ID_BIT_DEPTHS,      wxT("Bit Depth"), wxDefaultPosition, wxDefaultSize, 3, bit_depths, 0, wxRA_SPECIFY_ROWS);
    m_double_scale    = new wxCheckBox(this, ID_DOUBLE_SCALE,    wxT("Scale (2x)"));
    m_bilinear_filter = new wxCheckBox(this, ID_BILINEAR_FILTER, wxT("Bilinear Filter"));
    m_fullscreen      = new wxCheckBox(this, ID_FULLSCREEN,      wxT("Fullscreen"));
    m_vsync           = new wxCheckBox(this, ID_VSYNC,           wxT("VSync"));

    vbox1->Add(-1, 10);
    vbox1->Add(m_fullscreen);
    vbox1->Add(-1, 5);
    vbox1->Add(m_vsync);
    vbox1->Add(-1, 5);
    vbox1->Add(m_double_scale);
    vbox1->Add(-1, 5);
    vbox1->Add(m_bilinear_filter);

    hbox1->Add(5, -1);
    hbox1->Add(vbox1);
    hbox1->Add(20, -1);
    hbox1->Add(m_bit_depths);

    hbox2->Add(m_ok_button);
    hbox2->Add(10, -1);
    hbox2->Add(m_cancel_button);

    vbox2->Add(hbox1, 1, wxEXPAND);
    vbox2->Add(-1, 10);
    vbox2->Add(hbox2, 0, wxALIGN_CENTER);

    vbox->Add(vbox2, 1, wxEXPAND | wxALL, 10);

    SetSizer(vbox);

    Center();

    Connect(wxID_OK,     wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CDialogSphereGL::OnClickOk));
    Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CDialogSphereGL::OnClickCancel));
    Connect(ID_FULLSCREEN,   wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(CDialogSphereGL::OnClickFullscreen));
    Connect(ID_DOUBLE_SCALE, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(CDialogSphereGL::OnClickScale));

    LoadConfiguration();

}

//////////////////////////////////////////////////////////////////////////////////////
void
CDialogSphereGL::OnClickOk(wxCommandEvent& event)
{
    SaveConfiguration();
    EndModal(1);
}

//////////////////////////////////////////////////////////////////////////////////////
void
CDialogSphereGL::OnClickCancel(wxCommandEvent& event)
{
    EndModal(0);
}

//////////////////////////////////////////////////////////////////////////////////////
void
CDialogSphereGL::OnClickFullscreen(wxCommandEvent& event)
{
    if (m_fullscreen->IsChecked())
        m_vsync->Enable();
    else
        m_vsync->Enable(false);
}

//////////////////////////////////////////////////////////////////////////////////////
void
CDialogSphereGL::OnClickScale(wxCommandEvent& event)
{
    if (m_double_scale->IsChecked())
        m_bilinear_filter->Enable();
    else
        m_bilinear_filter->Enable(false);
}

//////////////////////////////////////////////////////////////////////////////////////
void
CDialogSphereGL::LoadDefaultConfiguration()
{
    m_fullscreen->SetValue(false);
    m_vsync->SetValue(false);
    m_double_scale->SetValue(false);
    m_bit_depths->SetSelection(2);

    m_vsync->Enable(false);
    m_bilinear_filter->Enable(false);
}

//////////////////////////////////////////////////////////////////////////////////////
void
CDialogSphereGL::LoadConfiguration()
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

    file.Read(m_drv_name + wxT("/BilinearFilter"), &bDummy, false);
    m_bilinear_filter->SetValue(bDummy);

    file.Read(m_drv_name + wxT("/BitDepth"), &lDummy, 32);

    if      (lDummy == 16)
        m_bit_depths->SetSelection(0);
    else if (lDummy == 24)
        m_bit_depths->SetSelection(1);
    else if (lDummy == 32)
        m_bit_depths->SetSelection(2);
    else
        m_bit_depths->SetSelection(0);


    if (!m_fullscreen->IsChecked())
        m_vsync->Enable(false);

    if (!m_double_scale->IsChecked())
        m_bilinear_filter->Enable(false);

}

//////////////////////////////////////////////////////////////////////////////////////
void
CDialogSphereGL::SaveConfiguration()
{
    wxString filename(wxGetCwd() + wxT("/system/video/") + m_drv_name + wxT(".cfg"));

    wxFileConfig file(wxEmptyString, wxEmptyString, filename, wxEmptyString, wxCONFIG_USE_LOCAL_FILE);

    file.Write(m_drv_name + wxT("/Fullscreen"),  m_fullscreen->IsChecked());
    file.Write(m_drv_name + wxT("/VSync"),       m_vsync->IsChecked());
    file.Write(m_drv_name + wxT("/Scale"), m_double_scale->IsChecked());
    file.Write(m_drv_name + wxT("/BilinearFilter"), m_bilinear_filter->IsChecked());
    file.Write(m_drv_name + wxT("/BitDepth"), 16 + 8 * m_bit_depths->GetSelection());

}








