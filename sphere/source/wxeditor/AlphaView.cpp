//#define WXWIN_COMPATIBILITY_EVENT_TYPES 1

#include "AlphaView.hpp"

/*todo:*/
static int s_AlphaViewID = wxID_HIGHEST + 700;


/*
BEGIN_MESSAGE_MAP(CAlphaView, CWnd)

  ON_WM_SIZE()
  ON_WM_VSCROLL()

END_MESSAGE_MAP()
*/

BEGIN_EVENT_TABLE(wAlphaView, wxWindow)
  EVT_SIZE(wAlphaView::OnSize)
  EVT_COMMAND_SCROLL(s_AlphaViewID, wAlphaView::OnVScroll)
END_EVENT_TABLE()


////////////////////////////////////////////////////////////////////////////////

wAlphaView::wAlphaView(wxWindow *parent, IAlphaViewHandler* handler)
: wxWindow(parent, -1)
, m_Alpha(0)
{
  wxWindow::wxWindow(parent, -1);
  m_Handler = handler;

  //wxBoxSizer *mainsizer = new wxBoxSizer(1);//wxVERTICAL);
    //mainsizer->Add(
  m_AlphaSlider = new wxSlider(this, s_AlphaViewID, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL);//, 1, wxGROW);// | wxALL, 5);
    //mainsizer->Add(
  m_AlphaStatic = new wxStaticText(this,  -1, "255", wxDefaultPosition, wxSize(16, 16) ,wxALIGN_RIGHT);//, 1, wxGROW);//, 1, wxALL, 5);
  //SetSizer(mainsizer);

  //mainsizer->SetSizeHints(this);

  UpdateSlider();
}


////////////////////////////////////////////////////////////////////////////////

wAlphaView::~wAlphaView()
{
  //delete m_AlphaSlider;
  //delete m_AlphaStatic;
  //m_AlphaSlider->Destroy();
  //m_AlphaStatic->Destroy();
  //Destroy();
}


/*todo:
////////////////////////////////////////////////////////////////////////////////

BOOL
CAlphaView::Create(IAlphaViewHandler* handler, CWnd* parent)
{
  m_Handler = handler;

  BOOL result = CWnd::Create(
    AfxRegisterWndClass(0, LoadCursor(NULL, IDC_ARROW), NULL, NULL),
    "",
    WS_CHILD | WS_VISIBLE,
    CRect(0, 0, 0, 0),
    parent,
    s_AlphaViewID++);

  m_AlphaSlider.Create(WS_CHILD | WS_VISIBLE | TBS_VERT, CRect(0, 0, 0, 0), this, 800);
  m_AlphaSlider.SetRange(0, 255);
  m_AlphaStatic.Create("", WS_CHILD | WS_VISIBLE | SS_CENTER, CRect(0, 0, 0, 0), this);

  m_Created = true;

  // make sure everything is in the correct place
  CRect rect;
  GetClientRect(rect);
  OnSize(0, rect.right, rect.bottom);

  UpdateSlider();

  return result;
}
*/

////////////////////////////////////////////////////////////////////////////////

void
wAlphaView::SetAlpha(byte alpha)
{
  m_Alpha = 255 - alpha;
  UpdateSlider();
}

////////////////////////////////////////////////////////////////////////////////

byte
wAlphaView::GetAlpha() const
{
  return 255 - m_Alpha;
}

////////////////////////////////////////////////////////////////////////////////

void
wAlphaView::UpdateSlider()
{
  m_AlphaSlider->SetValue(m_Alpha);

  char str[80];
  sprintf(str, "%d", 255 - m_Alpha);
  m_AlphaStatic->SetLabel(str);
}

////////////////////////////////////////////////////////////////////////////////

void
wAlphaView::OnSize(wxSizeEvent& event)
{
  wxSize clientsize = GetClientSize();
  //GetSizer()->SetDimension(0, 0, clientsize.GetWidth(), clientsize.GetHeight());
  const int STATIC_HEIGHT = 16;
  int cx = clientsize.x;
  int cy = clientsize.y;

  m_AlphaSlider->SetSize(0, 0, cx, cy - STATIC_HEIGHT);
  m_AlphaStatic->SetSize(0, cy - STATIC_HEIGHT, cx, STATIC_HEIGHT);
}

////////////////////////////////////////////////////////////////////////////////

void
wAlphaView::OnVScroll(wxScrollWinEvent &event)
{
  int alpha = m_Alpha;
  if(event.GetEventType() == wxEVT_SCROLL_TOP)          alpha = 0;
  if(event.GetEventType() == wxEVT_SCROLL_BOTTOM)       alpha = 255;
  if(event.GetEventType() == wxEVT_SCROLL_LINEUP)       alpha--;
  if(event.GetEventType() == wxEVT_SCROLL_LINEDOWN)     alpha++;
  if(event.GetEventType() == wxEVT_SCROLL_PAGEUP)       alpha -= 16;
  if(event.GetEventType() == wxEVT_SCROLL_PAGEDOWN)     alpha += 16;
  if(event.GetEventType() == wxEVT_SCROLL_THUMBTRACK)   alpha = m_AlphaSlider->GetValue();
  if(event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE) alpha = m_AlphaSlider->GetValue();

  if (alpha < 0) {
    alpha = 0;
  } else if (alpha > 255) {
    alpha = 255;
  }
  m_Alpha = alpha;

  UpdateSlider();
  m_Handler->AV_AlphaChanged(255 - m_Alpha);
}

////////////////////////////////////////////////////////////////////////////////
