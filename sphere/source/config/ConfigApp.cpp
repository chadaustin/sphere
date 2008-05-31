
#include "ConfigApp.h"
#include "ConfigFrame.h"


IMPLEMENT_APP(ConfigApp)

bool ConfigApp::OnInit()
{

    wxString error_message;

    CConfigFrame *config_frame = new CConfigFrame(wxT("Configure Sphere"));

    if (!config_frame->Initialize(error_message))
    {
        wxMessageDialog dial(NULL, error_message, wxT("Sphere Configuration"), wxOK | wxICON_ERROR);
        dial.ShowModal();

        return false;
    }

    config_frame->Show(true);


    return true;
}



















