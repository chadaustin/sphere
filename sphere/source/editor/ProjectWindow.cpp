#pragma warning(disable : 4786)  // identifier too long


#include "ProjectWindow.hpp"
#include "MainWindow.hpp"
#include "Project.hpp"
#include "FileDialogs.hpp"
#include "Scripting.hpp"
#include "../common/types.h"
#include "resource.h"


const int TreeID = 9865;

const int tiGameSettings = 100;
const int tiMaps         = 101;
const int tiSpritesets   = 102;
const int tiSounds       = 103;
const int tiScripts      = 104;
const int tiImages       = 105;
const int tiAnimations   = 106;
const int tiWindowStyles = 107;
const int tiFonts        = 108;


const int IDI_FILETYPE_BASE = 3;


BEGIN_MESSAGE_MAP(CProjectWindow, CPaletteWindow)

  ON_WM_SYSCOMMAND()
  ON_WM_SIZE()
  ON_WM_SETFOCUS()
  ON_WM_DROPFILES()

  ON_COMMAND(ID_PROJECTGROUP_INSERT, OnProjectGroupInsert)

  ON_COMMAND(ID_PROJECTITEM_OPEN,   OnProjectItemOpen)
  ON_COMMAND(ID_PROJECTITEM_DELETE, OnProjectItemDelete)

  ON_NOTIFY(TVN_KEYDOWN, TreeID, OnKeyDown)
  ON_NOTIFY(NM_DBLCLK,   TreeID, OnDoubleClick)
  ON_NOTIFY(NM_RCLICK,   TreeID, OnRightClick)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CProjectWindow::CProjectWindow(CMainWindow* main_window, CProject* project)
: m_MainWindow(main_window)
, m_Project(project)
, CPaletteWindow(NULL, "Project", CRect(0,0,200,200), true)
, m_htiMaps(NULL)
, m_htiSpritesets(NULL)
, m_htiScripts(NULL)
, m_htiSounds(NULL)
, m_htiFonts(NULL)
, m_htiWindowStyles(NULL)
, m_htiImages(NULL)
, m_htiAnimations(NULL)

, m_htiMapsExpanded(false)
, m_htiSpritesetsExpanded(false)
, m_htiScriptsExpanded(false)
, m_htiSoundsExpanded(false)
, m_htiFontsExpanded(false)
, m_htiWindowStylesExpanded(false)
, m_htiImagesExpanded(false)
, m_htiAnimationsExpanded(false)
{
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CProjectWindow::Create()
{
  /*char szProjectName[512];
  sprintf(szProjectName, "Project [%s]", m_Project->GetGameSubDirectory());	

  // create the window
  CPaletteWindow::Create(
    AfxRegisterWndClass(CS_NOCLOSE, NULL, NULL, AfxGetApp()->LoadIcon(IDI_PROJECT)),
    szProjectName,
    WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW);*/

  // create widgets
  m_TreeControl.Create(
    WS_VISIBLE | WS_CHILD | TVS_SHOWSELALWAYS | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS,
    CRect(0, 0, 0, 0),
    this,
    TreeID);
  m_TreeControl.SetFocus();

  HIMAGELIST image_list = ImageList_Create(16, 16, ILC_COLOR8 | ILC_MASK, 3, 1);
  ImageList_AddIcon(image_list, AfxGetApp()->LoadIcon(IDI_PV_GAME_SETTINGS));
  ImageList_AddIcon(image_list, AfxGetApp()->LoadIcon(IDI_PV_CLOSED_FOLDER));
  ImageList_AddIcon(image_list, AfxGetApp()->LoadIcon(IDI_PV_OPEN_FOLDER));  

  // add filetype icons
  ImageList_AddIcon(image_list, AfxGetApp()->LoadIcon(IDI_MAP));
  ImageList_AddIcon(image_list, AfxGetApp()->LoadIcon(IDI_SPRITESET));
  ImageList_AddIcon(image_list, AfxGetApp()->LoadIcon(IDI_SCRIPT));
  ImageList_AddIcon(image_list, AfxGetApp()->LoadIcon(IDI_SOUND));
  ImageList_AddIcon(image_list, AfxGetApp()->LoadIcon(IDI_FONT));
  ImageList_AddIcon(image_list, AfxGetApp()->LoadIcon(IDI_WINDOWSTYLE));
  ImageList_AddIcon(image_list, AfxGetApp()->LoadIcon(IDI_IMAGE));
  ImageList_AddIcon(image_list, AfxGetApp()->LoadIcon(IDI_ANIMATION));

  m_TreeControl.SetImageList(CImageList::FromHandle(image_list), TVSIL_NORMAL);

  // make sure they are in the right place
  RECT rect;
  GetClientRect(&rect);
  OnSize(0, rect.right, rect.bottom);

  Update();

  DragAcceptFiles();

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CProjectWindow::DestroyWindow()
{
  m_TreeControl.DestroyWindow();
  return CPaletteWindow::DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////

void
CProjectWindow::Update()
{
  m_Project->RefreshItems();

  // store the state of the items                                   
#define CHECK_EXPANDED(item)                                        \
  if (item) {                                                       \
    DWORD state = m_TreeControl.GetItemState(item, TVIS_EXPANDED);  \
    bool expanded = ((state & TVIS_EXPANDED) == TVIS_EXPANDED);     \
    item##Expanded = expanded;                                      \
  }

  CHECK_EXPANDED(m_htiMaps);
  CHECK_EXPANDED(m_htiSpritesets);
  CHECK_EXPANDED(m_htiScripts);
  CHECK_EXPANDED(m_htiSounds);
  CHECK_EXPANDED(m_htiFonts);
  CHECK_EXPANDED(m_htiWindowStyles);
  CHECK_EXPANDED(m_htiImages);
  CHECK_EXPANDED(m_htiAnimations);

#undef CHECK_EXPANDED

  // delete the current contents of the tree
  m_TreeControl.DeleteAllItems();

#define INSERT_ROOT(text, image, selected_image, item)          \
  m_TreeControl.InsertItem(                                     \
    TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE,   \
    text,                                                       \
    image,                                                      \
    selected_image,                                             \
    0,                                                          \
    0,                                                          \
    item,                                                       \
    TVI_ROOT,                                                   \
    TVI_LAST                                                    \
  );

  // insert top-level groups into the tree
                      INSERT_ROOT("Game Settings", 0, 0, tiGameSettings);
  m_htiMaps         = INSERT_ROOT("Maps",          1, 2, tiMaps);
  m_htiSpritesets   = INSERT_ROOT("Spritesets",    1, 2, tiSpritesets);
  m_htiScripts      = INSERT_ROOT("Scripts",       1, 2, tiScripts);
  m_htiSounds       = INSERT_ROOT("Sounds",        1, 2, tiSounds);
  m_htiFonts        = INSERT_ROOT("Fonts",         1, 2, tiFonts);
  m_htiWindowStyles = INSERT_ROOT("Window Styles", 1, 2, tiWindowStyles);
  m_htiImages       = INSERT_ROOT("Images",        1, 2, tiImages);
  m_htiAnimations   = INSERT_ROOT("Animations",    1, 2, tiAnimations);

#undef INSERT_ROOT

#define INSERT_ITEMS(hti, grouptype, image)                      \
  for (int i = 0; i < m_Project->GetItemCount(grouptype); i++) { \
    m_TreeControl.InsertItem(                                    \
      TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE,               \
      m_Project->GetItem(grouptype, i),                          \
      image, image, 0, 0, 0, hti, TVI_SORT                       \
    );                                                           \
  }

  // put the children in the tree
  INSERT_ITEMS(m_htiMaps,         GT_MAPS,         IDI_FILETYPE_BASE + 0);
  INSERT_ITEMS(m_htiSpritesets,   GT_SPRITESETS,   IDI_FILETYPE_BASE + 1);
  INSERT_ITEMS(m_htiScripts,      GT_SCRIPTS,      IDI_FILETYPE_BASE + 2);
  INSERT_ITEMS(m_htiSounds,       GT_SOUNDS,       IDI_FILETYPE_BASE + 3);
  INSERT_ITEMS(m_htiFonts,        GT_FONTS,        IDI_FILETYPE_BASE + 4);
  INSERT_ITEMS(m_htiWindowStyles, GT_WINDOWSTYLES, IDI_FILETYPE_BASE + 5);
  INSERT_ITEMS(m_htiImages,       GT_IMAGES,       IDI_FILETYPE_BASE + 6);
  INSERT_ITEMS(m_htiAnimations,   GT_ANIMATIONS,   IDI_FILETYPE_BASE + 7);

#undef INSERT_ITEMS

  // expand the groups if necessary
#define EXPAND_GROUP(item)                          \
  if (item##Expanded) {                             \
    m_TreeControl.Expand(item, TVE_EXPAND);         \
  }

  EXPAND_GROUP(m_htiMaps);
  EXPAND_GROUP(m_htiSpritesets);
  EXPAND_GROUP(m_htiScripts);
  EXPAND_GROUP(m_htiSounds);
  EXPAND_GROUP(m_htiFonts);
  EXPAND_GROUP(m_htiWindowStyles);
  EXPAND_GROUP(m_htiImages);
  EXPAND_GROUP(m_htiMaps);

#undef EXPAND_GROUP
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CProjectWindow::OnSysCommand(UINT id, LPARAM param)
{

  if (id == SC_CLOSE)
	{
		// do not close it damnit!
		// and oh, Ctrl-F4 calls close MDI window, if ya don't know.
		ShowWindow(SW_HIDE);
	}
	else
	{
    CWnd::OnSysCommand(id, param);
	}
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CProjectWindow::OnSize(UINT type, int cx, int cy)
{
  if (m_TreeControl.m_hWnd)
    m_TreeControl.MoveWindow(CRect(0, 0, cx, cy), TRUE);

  CPaletteWindow::OnSize(type, cx, cy);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CProjectWindow::OnSetFocus(CWnd* old_window)
{
  if (m_TreeControl.m_hWnd)
    m_TreeControl.SetFocus();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CProjectWindow::OnDropFiles(HDROP drop_info)
{
  UINT num_files = DragQueryFile(drop_info, 0xFFFFFFFF, NULL, 0);

  // add all files to the project
  for (int i = 0; i < num_files; i++) {

    char path[MAX_PATH];
    DragQueryFile(drop_info, i, path, MAX_PATH);

    // send message immediately (so path doesn't go out of scope)
    ::SendMessage(m_MainWindow->m_hWnd, WM_INSERT_PROJECT_FILE, 0, (LPARAM)path);
  }

  DragFinish(drop_info);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CProjectWindow::OnProjectGroupInsert()
{
  HTREEITEM item = m_TreeControl.GetSelectedItem();
  switch (m_TreeControl.GetItemData(item))
  {
    case tiMaps:         ::PostMessage(m_MainWindow->m_hWnd, WM_COMMAND, MAKEWPARAM(ID_PROJECT_INSERT_MAP,         0), 0); break;
    case tiSpritesets:   ::PostMessage(m_MainWindow->m_hWnd, WM_COMMAND, MAKEWPARAM(ID_PROJECT_INSERT_SPRITESET,   0), 0); break;
    case tiScripts:      ::PostMessage(m_MainWindow->m_hWnd, WM_COMMAND, MAKEWPARAM(ID_PROJECT_INSERT_SCRIPT,      0), 0); break;
    case tiSounds:       ::PostMessage(m_MainWindow->m_hWnd, WM_COMMAND, MAKEWPARAM(ID_PROJECT_INSERT_SOUND,       0), 0); break;
    case tiFonts:        ::PostMessage(m_MainWindow->m_hWnd, WM_COMMAND, MAKEWPARAM(ID_PROJECT_INSERT_FONT,        0), 0); break;
    case tiWindowStyles: ::PostMessage(m_MainWindow->m_hWnd, WM_COMMAND, MAKEWPARAM(ID_PROJECT_INSERT_WINDOWSTYLE, 0), 0); break;
    case tiImages:       ::PostMessage(m_MainWindow->m_hWnd, WM_COMMAND, MAKEWPARAM(ID_PROJECT_INSERT_IMAGE,       0), 0); break;
    case tiAnimations:   ::PostMessage(m_MainWindow->m_hWnd, WM_COMMAND, MAKEWPARAM(ID_PROJECT_INSERT_ANIMATION,   0), 0); break;
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CProjectWindow::OnProjectItemOpen()
{
  OnDoubleClick(NULL, NULL);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CProjectWindow::OnProjectItemDelete()
{
  HTREEITEM item = m_TreeControl.GetSelectedItem();
  if (item == NULL)
    return;

  // check to see if user clicked on an item in a folder
  HTREEITEM parent = m_TreeControl.GetParentItem(item);
  if (parent == NULL)
    return;

  int GroupType;
  switch (m_TreeControl.GetItemData(parent))
  {
    case tiMaps:         GroupType = GT_MAPS;         break;
    case tiSpritesets:   GroupType = GT_SPRITESETS;   break;
    case tiScripts:      GroupType = GT_SCRIPTS;      break;
    case tiSounds:       GroupType = GT_SOUNDS;       break;
    case tiFonts:        GroupType = GT_FONTS;        break;
    case tiWindowStyles: GroupType = GT_WINDOWSTYLES; break;
    case tiImages:       GroupType = GT_IMAGES;       break;
    case tiAnimations:   GroupType = GT_ANIMATIONS;   break;
  }

  // build file path
  char filename[MAX_PATH];
  strcpy(filename, m_Project->GetDirectory());
  strcat(filename, "\\");
  strcat(filename, m_Project->GetGroupDirectory(GroupType));
  strcat(filename, "\\");
  strcat(filename, m_TreeControl.GetItemText(item));

  std::string message = "This will permanently delete the file: \"" + std::string(filename) + "\"\nAre you sure you want to continue?";
  if (MessageBox(message.c_str(), "Delete Project Item", MB_YESNO) == IDNO)
    return;

  if (!DeleteFile(filename))
    MessageBox("Error: Could not delete file");

  m_Project->RefreshItems();
  Update();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CProjectWindow::OnKeyDown(NMHDR* notify, LRESULT* result)
{
  NMTVKEYDOWN* key = (NMTVKEYDOWN*)notify;

  switch (key->wVKey)
  {
    case VK_RETURN: 
      OnDoubleClick(NULL, NULL);
      break;

    case VK_INSERT:
      OnProjectGroupInsert();
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CProjectWindow::OnDoubleClick(NMHDR* /*notify*/, LRESULT* /*result*/)
{
  HTREEITEM item = m_TreeControl.GetSelectedItem();
  if (item == NULL)
    return;

  switch (m_TreeControl.GetItemData(item))
  {
    case tiGameSettings:
      // open game window
      m_MainWindow->OpenGameSettings();
      break;

    default:
    {
      // check to see if user clicked on an item in a folder
      HTREEITEM parent = m_TreeControl.GetParentItem(item);
      if (parent == NULL)
        return;

      int GroupType;
      switch (m_TreeControl.GetItemData(parent))
      {
        case tiMaps:         GroupType = GT_MAPS;         break;
        case tiSpritesets:   GroupType = GT_SPRITESETS;   break;
        case tiScripts:      GroupType = GT_SCRIPTS;      break;
        case tiSounds:       GroupType = GT_SOUNDS;       break;
        case tiFonts:        GroupType = GT_FONTS;        break;
        case tiWindowStyles: GroupType = GT_WINDOWSTYLES; break;
        case tiImages:       GroupType = GT_IMAGES;       break;
        case tiAnimations:   GroupType = GT_ANIMATIONS;   break;
      }

      // build file path
      char szFilename[MAX_PATH];
      strcpy(szFilename, m_Project->GetDirectory());
      strcat(szFilename, "\\");
      strcat(szFilename, m_Project->GetGroupDirectory(GroupType));
      strcat(szFilename, "\\");
      strcat(szFilename, m_TreeControl.GetItemText(item));

      m_MainWindow->OpenDocumentWindow(GroupType, szFilename);
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CProjectWindow::OnRightClick(NMHDR* notify, LRESULT* result)
{
  SelectItemAtCursor();
  HTREEITEM item = m_TreeControl.GetSelectedItem();
  if (item == NULL)
    return;

  // if the user right-clicked on a group, open the "group" popup menu
  switch (m_TreeControl.GetItemData(item))
  {
    case tiMaps:
    case tiSpritesets:
    case tiScripts:
    case tiSounds:
    case tiFonts:
    case tiWindowStyles:
    case tiImages:
    case tiAnimations:
    {
      CMenu Menu;
      Menu.LoadMenu(IDR_PROJECTGROUP);

      POINT Point;
      GetCursorPos(&Point);
      Menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, Point.x, Point.y, this);
      
      Menu.DestroyMenu();
      return;
    }
  }

  HTREEITEM parent = m_TreeControl.GetParentItem(item);
  if (parent == NULL)
    return;

  CMenu menu;
  menu.LoadMenu(IDR_PROJECTITEM);

  // get the mouse coordinates
  POINT Point;
  GetCursorPos(&Point);

  menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, Point.x, Point.y, this);
  
  menu.DestroyMenu();
}

////////////////////////////////////////////////////////////////////////////////

void
CProjectWindow::SelectItemAtCursor()
{
  // select the item the user right-clicked on
  POINT Point;
  GetCursorPos(&Point);
  ::ScreenToClient(m_TreeControl.m_hWnd, &Point);

  UINT uFlags;
  HTREEITEM item = m_TreeControl.HitTest(Point, &uFlags);

  // if we're on the item label, select it
  if (uFlags & TVHT_ONITEM)
    m_TreeControl.SelectItem(item);
  else
    m_TreeControl.SelectItem(NULL);
}


////////////////////////////////////////////////////////////////////////////////
