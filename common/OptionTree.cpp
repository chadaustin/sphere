#include <stdio.h>
#include <stdlib.h>
#include "OptionTree.hpp"



#define EDIT_BAR_WIDTH 120


/////////////
// General //
/////////////

////////////////////////////////////////////////////////////////////////////////

static void* DwordAlign(void* p)
{
  DWORD d = (DWORD)p;
  return (void*)((d + 3) & ~3);
}

////////////////////////////////////////////////////////////////////////////////

static void* WordAlign(void* p)
{
  DWORD d = (DWORD)p;
  return (void*)((d + 1) & ~1);
}

////////////////////////////////////////////////////////////////////////////////



/////////////////
// COptionTree //
/////////////////

////////////////////////////////////////////////////////////////////////////////

COptionTree::COptionTree()
: m_Head(NULL)
{
}

////////////////////////////////////////////////////////////////////////////////

COptionTree::~COptionTree()
{
  // destroy the linked list
  Node* p = m_Head;
  while (p)
  {
    Node* q = p;
    p = p->next;
    
    // delete node q
    delete[] q->name;
    delete q->sub_tree;
    delete q->label;
    delete q;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
COptionTree::AddTree(const char* name, COptionTree* tree)
{
  // create a node
  Node* node = new Node;
  node->next = NULL;
  node->name = new char[strlen(name) + 1];
  strcpy(node->name, name);
  node->is_sub_tree = true;

  node->sub_tree        = tree;
 
  node->data_type = OT_NONE;
  node->key       = 0;
  node->label     = NULL;

  // put the node on the end of the list
  AddNode(node);
}

////////////////////////////////////////////////////////////////////////////////

static char* newstring(const char* str)
{
  if (str == NULL)
    return NULL;
  else
  {
    char* s = new char[strlen(str) + 1];
    strcpy(s, str);
    return s;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
COptionTree::AddItem(const char* name, int key, enum EDataType data_type, UData data, const char* label)
{
  // create a node
  Node* node = new Node;
  node->next = NULL;
  node->name = new char[strlen(name) + 1];
  strcpy(node->name, name);
  node->is_sub_tree = false;

  node->sub_tree        = NULL;

  node->key       = key;
  node->data_type = data_type;
  node->data      = data;
  node->label     = newstring(label);

  // stick it in the list
  AddNode(node);
}

////////////////////////////////////////////////////////////////////////////////

EDataType
COptionTree::GetType(int key) const
{
  Node* p = m_Head;
  while (p)
  {
    if (p->is_sub_tree)
    {
      EDataType data_type = p->sub_tree->GetType(key);
      if (data_type != OT_NONE)
        return data_type;
    }
    else
    {
      if (p->key == key)
        return p->data_type;
    }

    p = p->next;
  }

  return OT_NONE;
}

////////////////////////////////////////////////////////////////////////////////

const UData*
COptionTree::GetItem(int key) const
{
  Node* p = m_Head;
  while (p)
  {
    if (p->is_sub_tree)
    {
      const UData* data = p->sub_tree->GetItem(key);
      if (data)
        return data;
    }
    else
    {
      if (p->key == key)
        return &p->data;
    }

    p = p->next;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

void
COptionTree::AddNode(Node* node)
{
  if (m_Head == NULL)
    m_Head = node;
  else
  {
    Node* p = m_Head;
    while (p->next)
      p = p->next;
    p->next = node;
  }
}

////////////////////////////////////////////////////////////////////////////////



///////////////////////
// COptionTreeDialog //
///////////////////////


////////////////////////////////////////////////////////////////////////////////

COptionTreeDialog::COptionTreeDialog()
: m_Instance(NULL)
, m_Window(NULL)
, m_TreeControl(NULL)
, m_OldTreeProc(NULL)
, m_Tree(NULL)
{
  InitCommonControls();
}

////////////////////////////////////////////////////////////////////////////////

COptionTreeDialog::~COptionTreeDialog()
{
  delete m_Tree;
}

////////////////////////////////////////////////////////////////////////////////

void
COptionTreeDialog::SetTree(COptionTree* tree)
{
  delete m_Tree;
  m_Tree = tree;
}

////////////////////////////////////////////////////////////////////////////////

bool
COptionTreeDialog::Execute(HINSTANCE instance, HWND parent, const char* caption)
{
  m_Instance = instance;

  // create dialog box template
  void* memory_block = new BYTE[4096];
  memset(memory_block, 0, 4096);
  WORD* wc;

  // fill the dialog template
  DLGTEMPLATE* dt = (DLGTEMPLATE*)memory_block;
  dt->style           = WS_POPUP | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_CLIPCHILDREN | DS_MODALFRAME | DS_CENTER | DS_SETFONT;
  dt->dwExtendedStyle = WS_EX_CLIENTEDGE;
  dt->cdit            = 0;
  dt->x               = 0;
  dt->y               = 0;
  dt->cx              = 200;
  dt->cy              = 200;

  // add menu / class / title arrays
  wc = (WORD*)WordAlign(dt + 1);
  *wc++ = 0;  // no menu
  *wc++ = 0;  // predefined dialog box class
  wc += MultiByteToWideChar(CP_ACP, 0, caption, -1, wc, strlen(caption) + 1);
  *wc++ = 8;  // size 8 font
  wc += MultiByteToWideChar(CP_ACP, 0, "MS Shell Dlg", -1, wc, strlen("MS Shell Dlg") + 1);

  // execute the dialog box
  DialogBoxIndirectParam(instance, dt, parent, DialogProc, (LPARAM)this);
  delete[] memory_block;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK
COptionTreeDialog::DialogProc(HWND window, UINT message, WPARAM /*wparam*/, LPARAM lparam)
{
  COptionTreeDialog* This = (COptionTreeDialog*)GetWindowLong(window, GWL_USERDATA);

  switch (message)
  {
    case WM_CREATE:
      return TRUE;

    ////////////////////////////////////////////////////////////////////////////

    case WM_INITDIALOG:
      This = (COptionTreeDialog*)lparam;
      SetWindowLong(window, GWL_USERDATA, (LONG)This);
      This->m_Window = window;

      This->OnCreate();
      
      // WM_SIZE is sent after WM_CREATE and before WM_INITDIALOG so we need
      // to make sure everything is sized correctly
      RECT ClientRect;
      GetClientRect(window, &ClientRect);
      This->OnSize(ClientRect.right, ClientRect.bottom);
      return FALSE;

    ////////////////////////////////////////////////////////////////////////////

    case WM_DESTROY:
      This->OnDestroy();
      return TRUE;

    ////////////////////////////////////////////////////////////////////////////

    case WM_CLOSE:
      EndDialog(window, 0);
      return TRUE;

    ////////////////////////////////////////////////////////////////////////////

    case WM_SIZE:
      This->OnSize(LOWORD(lparam), HIWORD(lparam));
      return TRUE;

    ////////////////////////////////////////////////////////////////////////////

    case WM_NOTIFY:
    {
      // if item was double-clicked or enter was pressed...
      NMHDR* nmh = (NMHDR*)lparam;
      if (nmh->hwndFrom == This->m_TreeControl &&
          (nmh->code == NM_DBLCLK || nmh->code == NM_RETURN))
      {
        This->OnItemClick();
        return TRUE;
      }

      // if item was expanded...
      if (nmh->hwndFrom == This->m_TreeControl &&
          nmh->code == TVN_ITEMEXPANDED)
      {
        This->OnItemExpand();
        return TRUE;
      }


      return FALSE;
    }

    ////////////////////////////////////////////////////////////////////////////

    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      BeginPaint(window, &ps);
      This->OnPaint(ps.hdc);
      EndPaint(window, &ps);
      return TRUE;
    }

    ////////////////////////////////////////////////////////////////////////////

    case WM_ERASEBKGND:
      // don't render the background
      return TRUE;

    ////////////////////////////////////////////////////////////////////////////

    default:
      return FALSE;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
COptionTreeDialog::OnCreate()
{
  // create the tree control
  m_TreeControl = CreateWindow(
    WC_TREEVIEW,
    "",
    WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT,
    0, 0, 0, 0,
    m_Window,
    NULL,
    m_Instance,
    NULL);

  if (m_TreeControl == NULL)
    MessageBox(m_Window, "Could not create tree control", "Option Tree", MB_OK);

  // associate this object with the tree
  SetWindowLong(m_TreeControl, GWL_USERDATA, (LONG)this);
  
  // subclass the tree
  m_OldTreeProc = (WNDPROC)GetWindowLong(m_TreeControl, GWL_WNDPROC);
  SetWindowLong(m_TreeControl, GWL_WNDPROC, (LONG)NewTreeProc);

  SetFocus(m_TreeControl);
  
  // if there is a tree, initialize the tree control
  if (m_Tree)
    PopulateTreeControl(m_Tree);
}

////////////////////////////////////////////////////////////////////////////////

void
COptionTreeDialog::OnDestroy()
{
  DestroyWindow(m_TreeControl);
}

////////////////////////////////////////////////////////////////////////////////

void
COptionTreeDialog::OnSize(int cx, int cy)
{
  if (m_TreeControl)
  {
    MoveWindow(m_TreeControl, 0, 0, cx - EDIT_BAR_WIDTH, cy, TRUE);
    InvalidateRect(m_Window, NULL, TRUE);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
COptionTreeDialog::OnItemClick()
{
  // get the currently-selected item
  HTREEITEM item = TreeView_GetSelection(m_TreeControl);
  if (item == NULL)
    return;

  // get the node associated with the item
  TVITEM tvitem;
  tvitem.mask = TVIF_HANDLE | TVIF_PARAM;
  tvitem.hItem = item;
  TreeView_GetItem(m_TreeControl, &tvitem);

  // convert the item's parameter to a node
  COptionTree::Node* node = (COptionTree::Node*)tvitem.lParam;

  // show the dialog box if the data type is valid
  switch (node->data_type)
  {
    case OT_INTEGER:
    case OT_STRING:
    case OT_FLOATING:
      ItemDialogBox(node);
      InvalidateRect(m_Window, NULL, TRUE);
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
COptionTreeDialog::OnItemExpand()
{
}

////////////////////////////////////////////////////////////////////////////////

void
COptionTreeDialog::OnPaint(HDC dc)
{
  // get the client size
  RECT ClientRect;
  GetClientRect(m_Window, &ClientRect);

  // get tree control item height
  int item_height = TreeView_GetItemHeight(m_TreeControl);

  HFONT  font          = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
  HFONT  old_font      = (HFONT)SelectObject(dc, font);
  HPEN   pen           = (HPEN)GetStockObject(BLACK_PEN);
  HBRUSH valid_brush   = CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));
  HBRUSH invalid_brush = CreateSolidBrush(RGB(0x00, 0x00, 0x00));

  for (int i = 0; i < ClientRect.bottom / item_height + 1; i++)
  {
    // see what item is in this row
    TVHITTESTINFO hti;
    hti.pt.x = 0;
    hti.pt.y = i * item_height;
    TreeView_HitTest(m_TreeControl, &hti);

    HBRUSH brush = invalid_brush;
    COptionTree::Node* node = NULL;

    // if there is an item, see if it is an item or a subtree
    if (hti.hItem != NULL)
    {
      TVITEM item = { TVIF_PARAM, hti.hItem };
      TreeView_GetItem(m_TreeControl, &item);
      node = (COptionTree::Node*)item.lParam;

      if (node->is_sub_tree == false)
        brush = valid_brush;
    }

    // calculate the rect in which to draw the value
    RECT Rect = {
      ClientRect.right - EDIT_BAR_WIDTH,
      i * item_height,
      ClientRect.right,
      (i + 1) * item_height
    };

    // draw the field
    SaveDC(dc);
    SelectObject(dc, pen);
    SelectObject(dc, brush);

    Rectangle(dc, Rect.left, Rect.top, Rect.right, Rect.bottom);
    if (node)
    {
      char string[81];
      GetNodeText(node, string);
      if (node->label)
      {
        strcat(string, " ");
        strcat(string, node->label);
      }
      TextOut(dc, Rect.left + 1, Rect.top + 1, string, strlen(string));
    }
    
    RestoreDC(dc, -1);
  }

  DeleteObject(pen);
  DeleteObject(valid_brush);
  DeleteObject(invalid_brush);
  SelectObject(dc, old_font);
}

////////////////////////////////////////////////////////////////////////////////

void
COptionTreeDialog::GetNodeText(COptionTree::Node* node, char* text)
{
  switch (node->data_type)
  {
    case OT_INTEGER:  sprintf(text, "%d", node->data.integer);  break;
    case OT_STRING:   sprintf(text, "%s", node->data.str);      break;
    case OT_FLOATING: sprintf(text, "%g", node->data.floating); break;
    default:          strcpy(text, "");                         break;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
COptionTreeDialog::PopulateTreeControl(COptionTree* tree, HTREEITEM parent)
{
  // step through the linked list inserting items into the tree control
  COptionTree::Node* p = tree->m_Head;
  while (p)
  {
    char name[520];
    strcpy(name, p->name);

    TVINSERTSTRUCT is;
    is.hParent      = parent;
    is.hInsertAfter = TVI_LAST;
    is.item.mask    = TVIF_TEXT | TVIF_PARAM;
    is.item.pszText = name;
    is.item.lParam  = (LPARAM)p;

    // insert the item into the tree
    HTREEITEM item = TreeView_InsertItem(m_TreeControl, &is);
    if (p->is_sub_tree)
      PopulateTreeControl(p->sub_tree, item);

    // advance to the next part of the linked list
    p = p->next;
  }
}

////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK
COptionTreeDialog::NewTreeProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  COptionTreeDialog* This = (COptionTreeDialog*)GetWindowLong(window, GWL_USERDATA);
  switch (message)
  {
    // if the tree control gets painted, update the sidebar too
    case WM_PAINT:
      InvalidateRect(This->m_Window, NULL, TRUE);
      // pass through to default message handler

    default: 
      return CallWindowProc(This->m_OldTreeProc, window, message, wparam, lparam);
  }
}

////////////////////////////////////////////////////////////////////////////////

#define IDC_EDIT 200

void
COptionTreeDialog::ItemDialogBox(COptionTree::Node* node)
{
  void* memory_block = new BYTE[4096];  // this must be big enough to hold the dialog resource
  memset(memory_block, 0, 4096);
  WORD* wc;
    
  // fill the dialog template
  DLGTEMPLATE* dt = (DLGTEMPLATE*)memory_block;
  dt->style = WS_POPUP | WS_BORDER | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | DS_CENTER | DS_SETFONT;
  dt->cdit  = 3;
  dt->x     = 0;
  dt->y     = 0;
  dt->cx    = 144;
  dt->cy    = 44;

  // add menu / class / title arrays
  wc = (WORD*)WordAlign(dt + 1);
  *wc++ = 0;  // no menu
  *wc++ = 0;  // predefined dialog box class
  wc += MultiByteToWideChar(CP_ACP, 0, node->name, -1, wc, strlen(node->name) + 1);
  *wc++ = 8;  // size 8 font
  wc += MultiByteToWideChar(CP_ACP, 0, "MS Shell Dlg", -1, wc, strlen("MS Shell Dlg") + 1);
  

  // add controls
  DLGITEMTEMPLATE* dit;

  // add OK button
  dit = (DLGITEMTEMPLATE*)DwordAlign(wc);
  dit->style = WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON;
  dit->x     = 14;
  dit->y     = 26;
  dit->cx    = 50;
  dit->cy    = 14;
  dit->id    = IDOK;
  
  wc = (WORD*)WordAlign(dit + 1);
  *wc++ = 0xFFFF;
  *wc++ = 0x0080;  // button class
  wc += MultiByteToWideChar(CP_ACP, 0, "OK", -1, wc, 3);
  wc = (WORD*)WordAlign(wc);
  *wc++ = 0;       // no creation data
  
  // add Cancel button
  dit = (DLGITEMTEMPLATE*)DwordAlign(wc);
  dit->style = WS_CHILD | WS_VISIBLE;
  dit->x     = 80;
  dit->y     = 26;
  dit->cx    = 50;
  dit->cy    = 14;
  dit->id    = IDCANCEL;

  wc = (WORD*)WordAlign(dit + 1);
  *wc++ = 0xFFFF;
  *wc++ = 0x0080;  // button class
  wc += MultiByteToWideChar(CP_ACP, 0, "Cancel", -1, wc, 7);
  wc = (WORD*)WordAlign(wc);
  *wc++ = 0;       // no creation data

  // add Edit control
  dit = (DLGITEMTEMPLATE*)DwordAlign(wc);
  dit->style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL;
  dit->x     = 14;
  dit->y     = 6;
  dit->cx    = 116;
  dit->cy    = 12;
  dit->id    = IDC_EDIT;

  wc = (WORD*)WordAlign(dit + 1);
  *wc++ = 0xFFFF;
  *wc++ = 0x0081;  // edit class
  *wc++ = 0;       // no default text
  *wc++ = 0;       // no creation data

  // display the dialog box and give it the node so it can modify it
  DialogBoxIndirectParam(m_Instance, dt, m_Window, ItemDialogProc, (LPARAM)node);

  delete[] memory_block;
}

////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK
COptionTreeDialog::ItemDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {
    case WM_INITDIALOG:
    {
      // get the node and attach it to the window
      COptionTree::Node* node = (COptionTree::Node*)lparam;
      SetWindowLong(window, GWL_USERDATA, (LONG)node);

      // create necessary dialog controls
      char edit_text[80];
      GetNodeText(node, edit_text);
      SetDlgItemText(window, IDC_EDIT, edit_text);

      // give the edit control focus and select all of the test
      SetFocus(GetDlgItem(window, IDC_EDIT));
      SendDlgItemMessage(window, IDC_EDIT, EM_SETSEL, 0, -1);

      return FALSE;
    }

    ////////////////////////////////////////////////////////////////////////////

    case WM_COMMAND:
      switch (LOWORD(wparam))
      {
        case IDOK:
        {
          // get the string from the edit control
          char edit_text[81];
          edit_text[80] = 0;
          GetDlgItemText(window, IDC_EDIT, edit_text, 80);

          // place the data back into the node
          COptionTree::Node* node = (COptionTree::Node*)GetWindowLong(window, GWL_USERDATA);
          switch (node->data_type)
          {
            case OT_INTEGER:
              node->data.integer = atoi(edit_text);
              break;

            case OT_STRING:
              strncpy(node->data.str, edit_text, 80);
              break;

            case OT_FLOATING:
              node->data.floating = atof(edit_text);
              break;
          }

          EndDialog(window, IDOK);
          return TRUE;
        }

        case IDCANCEL:
          EndDialog(window, IDCANCEL);
          return TRUE;
      }
      return FALSE;

    ////////////////////////////////////////////////////////////////////////////

    default:
      return FALSE;
  }
}

////////////////////////////////////////////////////////////////////////////////
