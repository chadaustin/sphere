#pragma warning(disable : 4786)

#include <Scintilla.h>
#include <SciLexer.h>
#include "Editor.hpp"
#include "ScriptWindow.hpp"
#include "FileDialogs.hpp"
#include "NumberDialog.hpp"
//#include "Project.hpp"
#include "Scripting.hpp"
#include "FileSystem.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include "resource.h"

//#include "MainWindow.hpp"
#include "LineSorterDialog.hpp"
#include "LineSorter.hpp"

#include <afxdlgs.h>

const int ID_EDIT = 900;
const UINT s_FindReplaceMessage = ::RegisterWindowMessage(FINDMSGSTRING);

////////////////////////////////////////////////////////////////////////////////

 static const char* sFunctionDefinitions =
  "Abort(message) "
  "ApplyColorMask(color) "
  "Array.concat(value) "
  "Array.join(seperator) "
  "Array.pop() "
  "Array.push(value) "
  "Array.reverse() "
  "Array.shift() "
  "Array.slice(start_pos,end_pos) "
  "Array.sort(order_func) "
  "Array.splice(start_pos,delete_count,value) "
  //"Array.toString() "
  "Array.unshift(value) "
  "AreKeysLeft() "
  "AreZonesAt(map_x,map_y,layer) "
  "AttachCamera(person_name) "
  "AttachInput(person_entity) "
  "AttachPlayerInput(person_entity,player_index) "
  "BLEND "
  "BindKey(key,onkeydown,onkeyup) "
  "BlendColors(c1,c2) "
  "BlendColorsWeighted(c1,c2,c1_weight,c2_weight) "
  "COMMAND_ANIMATE "
  "COMMAND_FACE_EAST "
  "COMMAND_FACE_NORTH "
  "COMMAND_FACE_NORTHEAST "
  "COMMAND_FACE_NORTHWEST "
  "COMMAND_FACE_SOUTH "
  "COMMAND_FACE_SOUTHEAST "
  "COMMAND_FACE_SOUTHWEST "
  "COMMAND_FACE_WEST "
  "COMMAND_MOVE_EAST "
  "COMMAND_MOVE_NORTH "
  "COMMAND_MOVE_SOUTH "
  "COMMAND_MOVE_WEST "
  "COMMAND_WAIT "
  "CallDefaultMapScript(which) "
  "CallMapScript(which) "
  "CallPersonScript(name,which) "
  "ChangeMap(map) " 
  "ClearPersonCommands(name) "
  "CreateByteArray(size) "
  "CreateByteArrayFromString(string) "
  "CreateColor(r,g,b,a) "
  "CreateColorMatrix(rn,rr,rg,rb,gn,gr,gg,gb,bn,br,bg,bb) "
  "CreatePerson(name,spriteset,destroy_with_map) "
  "CreateStringFromByteArray(array) "
  "CreateStringFromCode(code) "
  "CreateSurface(width,height,color) "
  "Date.getDate() "
  "Date.getDay() "
  "Date.getFullYear() "
  "Date.getHours() "
  "Date.getMilliseconds() "
  "Date.getMinutes() "
  "Date.getMonth() "
  "Date.getSeconds() "
  "Date.getTime() "
  "Date.getTimezoneOffset() "
  "Date.getUTCDate() "
  "Date.getUTCDate() "
  "Date.getUTCFullYear() "
  "Date.getUTCHours() "
  "Date.getUTCMilliseconds() "
  "Date.getUTCMinutes() "
  "Date.getUTCMonth() "
  "Date.getUTCSeconds() "
  "Date.getYear() "
  "Date.parse(date_str) "
  "Date.setDate(day_of_month) "
  "Date.setFullYear(year) "
  "Date.setHours(hours) "
  "Date.setMilliseconds(millis) "
  "Date.setMinutes(minutes) "
  "Date.setMonth(month) "
  "Date.setSeconds(seconds) "
  "Date.setTime(milliseconds) "
  "Date.setUTCDate(day_of_month) "
  "Date.setUTCFullYear(year) "
  "Date.setUTCHours(hours) "
  "Date.setUTCMilliseconds(millis) "
  "Date.setUTCMinutes(minutes) "
  "Date.setUTCMonth(month) "
  "Date.setUTCSeconds(seconds) "
  "Date.setYear(year) "
  "Date.toGMTString() "
  "Date.toLocaleString() "
  //"Date.toString() "
  "Date.toUTCString() "
  "Date.UTC(year,month,day,hours,minutes,seconds,ms) "
  //"Date.valueOf() "
  "DestroyPerson(name) "
  "DetachCamera() "
  "DetachInput() "
  "DetachPlayerInput(person_entity) "
  "EvaluateScript(script) "
  "EvaluateSystemScript(script) "
  "ExecuteGame(directory) "
  "ExecuteTrigger(map_x,map_y,layer) "
  "ExecuteZoneScript(zone) "
  "ExecuteZones(map_x,map_y,layer) "
  "Exit() "
  "ExitMapEngine() "
  "FlipScreen() "
  "FollowPerson(name,leader,pixels) "
  "GarbageCollect() "
  "GetCameraPerson() "
  "GetCameraX() "
  "GetCameraY() "
  "GetCurrentMap() "
  "GetCurrentPerson() "
  "GetCurrentZone() "
  "GetFileList(directory) "
  "GetFrameRate() "  
  "GetGameList() "
  "GetInputPerson() "
  "GetJoystickX(joy) "
  "GetJoystickY(joy) "
  "GetKey() "
  "GetKeyString(key,shift) "
  "GetLayerHeight(layer) "
  "GetLayerMask(layer) "
  "GetLayerName(layer) "
  "GetLayerWidth(layer) "
  "GetLocalAddress() "
  "GetLocalName() "
  "GetMapEngineFrameRate() "
  "GetMouseX() "
  "GetMouseY() "
  "GetNextAnimatedTile(tile) "
  "GetNumJoystickButtons(joy) "
  "GetNumJoysticks() "
  "GetNumLayers() "
  "GetNumTiles() "
  "GetNumZones() "
  "GetObstructingPerson(name,x,y) "
  "GetObstructingTile(name,x,y) "
  "GetPersonAngle(name) "
  "GetPersonBase(name) "
  "GetPersonData(name) "
  "GetPersonDirection(name) "
  "GetPersonFrame(name) "
  "GetPersonFrameRevert(name) "
  "GetPersonIgnoreList(person) "
  "GetPersonLayer(name) "
  "GetPersonList() "
  "GetPersonMask(name) "
  "GetPersonSpeedX(name) "
  "GetPersonSpeedY(name) "
  "GetPersonSpriteset(name) "
  "GetPersonValue(name,key) "
  "GetPersonX(name) "
  "GetPersonXFloat(name) "
  "GetPersonY(name) "
  "GetPersonYFloat(name) "
  "GetScreenHeight() "
  "GetScreenWidth() "
  "GetSystemArrow() "
  "GetSystemDownArrow() "
  "GetSystemFont() "
  "GetSystemUpArrow() "
  "GetSystemWindowStyle() "
  "GetTalkActivationKey() "
  "GetTalkDistance() "
  "GetTile(x,y,layer) "
  "GetTileDelay(tile) "
  "GetTileHeight() "
  "GetTileImage(tile_index) "
  "GetTileSurface(tile_index) "
  "GetTileWidth() "
  "GetTime() "
  "GetVersion() "
  "GetVersionString() "
  "GetZoneHeight(zone) "
  "GetZoneLayer(zone) "
  "GetZoneWidth(zone) "
  "GetZoneX(zone) "
  "GetZoneY(zone) "
  "GrabImage(x,y,w,h) "
  "GrabSurface(x,y,w,h) "
  "GradientLine(x1,y1,x2,y2,color1,color2) "
  "GradientRectangle(x,y,w,h,c_ul,c_ur,c_lr,c_ll) "
  "GradientTriangle(x1,y1,x2,y2,x3,y3,c1,c2,c3) "
  "HashByteArray(byte_array) "
  "HashFromFile(filename) "
  "IgnorePersonObstructions(person,ignore) "
  "IgnoreTileObstructions(person,ignore) "
  "IsAnyKeyPressed() "
  "IsCameraAttached() "
  "IsCommandQueueEmpty(name) "
  "IsIgnoringPersonObstructions(person) "
  "IsIgnoringTileObstructions(person) "
  "IsInputAttached() "
  "IsJoystickButtonPressed(joy,button) "
  "IsKeyPressed(key) "
  "IsLayerVisible(layer) "
  "IsMapEngineRunning() "
  "IsMouseButtonPressed(button) "
  "IsPersonObstructed(name,x,y) "
  "IsTriggerAt(map_x,map_y,layer) "
  "IsZoneAt(map_x,map_y,layer) "
  "Line(x1,y1,x2,y2,color) "
  "ListenOnPort(port) "
  "LoadAnimation(filename) "
  "LoadFont(filename) "
  "LoadImage(filename) "
  "LoadSound(filename,streaming) "
  "LoadSpriteset(filename) "
  "LoadSurface(filename) "
  "LoadWindowStyle(filename) "
  "MapEngine(map,fps) "
  "MapToScreenX(layer,x) "
  "MapToScreenY(layer,y) "
  "Math.E "
  "Math.LN10 "
  "Math.LN2 "
  "Math.LOG10E "
  "Math.LOG2E "
  "Math.PI "
  "Math.SQRT1_2 "
  "Math.SQRT2 "
  "Math.abs(value) "
  "Math.acos(v) "
  "Math.asin(v) "
  "Math.atan(v) "
  "Math.atan2(v) "
  "Math.ceil(value) "
  "Math.cos(radian) "
  "Math.exp(v) "
  "Math.floor(value) "
  "Math.log(v) "
  "Math.max(va,vb) "
  "Math.min(va,vb) "
  "Math.pow(x,y) "
  "Math.random() "
  "Math.round(value) "
  "Math.sin(radian) "
  "Math.sqrt(value) "
  "Math.tan(radian) "
  "OpenAddress(address,port) "
  "OpenFile(filename) "
  "OpenLog(filename) "
  "OpenRawFile(filename,writeable) "
  "Point(x,y,color) "
  "QueuePersonCommand(name,command,immediate) "
  "QueuePersonScript(name,script,immediate) "
  "REPLACE "
  "Rectangle(x,y,w,h,c) "
  "RenderMap() "
  "ReplaceTilesOnLayer(layer,oldtile,newtile) "
  "RequireScript(script) "
  "RequireSystemScript(script) "
  "RestartGame() "
  "SCRIPT_COMMAND_GENERATOR "
  "SCRIPT_ON_ACTIVATE_TALK "
  "SCRIPT_ON_ACTIVATE_TOUCH "
  "SCRIPT_ON_CREATE "
  "SCRIPT_ON_DESTROY "
  "SCRIPT_ON_ENTER_MAP "
  "SCRIPT_ON_LEAVE_MAP "
  "SCRIPT_ON_LEAVE_MAP_EAST "
  "SCRIPT_ON_LEAVE_MAP_NORTH "
  "SCRIPT_ON_LEAVE_MAP_SOUTH "
  "SCRIPT_ON_LEAVE_MAP_WEST "
  "ScreenToMapX(layer,x) "
  "ScreenToMapY(layer,y) "
  "SetCameraX(x) "
  "SetCameraY(y) "
  "SetClippingRectangle(x,y,w,h) "
  "SetColorMask(color,num_frames) "
  "SetDelayScript(num_frames,script) "
  "SetFrameRate(fps) "
  "SetLayerMask(layer,mask) "
  "SetLayerRenderer(layer,script) "
  "SetLayerScaleFactorX(layer_index,factor_x) "
  "SetLayerScaleFactorY(layer_index,factor_y) "
  "SetLayerVisible(layer,visible) "
  "SetMapEngineFrameRate(fps) "
  "SetMousePosition(x,y) "
  "SetNextAnimatedTile(tile,next_tile) "
  "SetPersonAngle(name,angle) "
  "SetPersonData(name,data) "
  "SetPersonDirection(name,direction) "
  "SetPersonFrame(name,frame) "
  "SetPersonFrameRevert(name,delay) "
  "SetPersonIgnoreList(person,ignore_list) "
  "SetPersonLayer(name,layer) "
  "SetPersonMask(name,color) "
  "SetPersonScaleAbsolute(name,width,height) "
  "SetPersonScaleFactor(name,scale_w,scale_h) "
  "SetPersonScript(name,which,script) "
  "SetPersonSpeed(name,speed) "
  "SetPersonSpeedXY(name,speed_x,speed_y) "
  "SetPersonSpriteset(name,spriteset) "
  "SetPersonValue(name,key,value) "
  "SetPersonX(name,x) "
  "SetPersonXYFloat(name,x,y) "
  "SetPersonY(name,y) "
  "SetRenderScript(script) "
  "SetTalkActivationKey(key) "
  "SetTalkDistance(pixels) "
  "SetTile(x,y,layer,tile) "
  "SetTileDelay(tile,delay) "
  "SetTileImage(tile_index,image_object) "
  "SetTileSurface(tile_index,surface_object) "
  "SetUpdateScript(script) "
  "SetZoneLayer(zone,layer) "
  "String.charAt(pos) "
  "String.charCodeAt(pos) "
  "String.concat(str) "
  "String.fromCharCode(code) "
  "String.indexOf(str,start_pos) "
  "String.lastIndexOf(str,start_pos) "
  "String.length "
  "String.match(regexp) "
  "String.replace(regexp,replacement) "
  "String.search(regexp) "
  "String.slice(start_pos,end_pos) "
  "String.split(delimiter) "
  "String.substr(start_pos,length) "
  "String.substring(start_pos,end_pos) "
  "String.toLowerCase() "
  "String.toUpperCase() "
  "Triangle(x1,y1,x2,y2,x3,y3,c) "
  "UnbindKey(key) "
  "UpdateMapEngine()";

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CScriptWindow, CSaveableDocumentWindow)

  ON_WM_SIZE()
  ON_WM_SETFOCUS()
  ON_WM_CONTEXTMENU()

  ON_COMMAND(ID_SCRIPT_CHECKSYNTAX,      OnScriptCheckSyntax)
  ON_COMMAND(ID_SCRIPT_FIND,             OnScriptFind)
  ON_COMMAND(ID_SCRIPT_REPLACE,          OnScriptReplace)
  ON_COMMAND(ID_SCRIPT_GOTOLINE,         OnScriptGotoLine)

  ON_COMMAND(ID_SCRIPT_OPTIONS_SET_FONT,      OnOptionsSetScriptFont)
  ON_COMMAND(ID_SCRIPT_OPTIONS_TOGGLE_COLORS, OnOptionsToggleColors)
  ON_COMMAND(ID_SCRIPT_OPTIONS_SET_TAB_SIZE,  OnOptionsSetTabSize)
  ON_COMMAND(ID_SCRIPT_OPTIONS_TOGGLE_LINE_NUMBERS,  OnOptionsToggleLineNumbers)
  ON_COMMAND(ID_SCRIPT_OPTIONS_SHOW_WHITESPACE, OnOptionsShowWhitespace)
  ON_COMMAND(ID_SCRIPT_OPTIONS_WORD_WRAP, OnOptionsWordWrap)
  ON_COMMAND(ID_SCRIPT_OPTIONS_TOGGLE_AUTOCOMPLETE, OnOptionsToggleAutoComplete)
  
  ON_COMMAND(ID_SCRIPT_OPTIONS_SELECTION_STREAM, OnOptionsSelectionStream)
  ON_COMMAND(ID_SCRIPT_OPTIONS_SELECTION_RECTANGLE, OnOptionsSelectionRectangle)
  ON_COMMAND(ID_SCRIPT_OPTIONS_SELECTION_LINE, OnOptionsSelectionLine)
  ON_UPDATE_COMMAND_UI(ID_SCRIPT_OPTIONS_SELECTION_STREAM, OnUpdateOptionsSelectionStream)
  ON_UPDATE_COMMAND_UI(ID_SCRIPT_OPTIONS_SELECTION_RECTANGLE, OnUpdateOptionsSelectionRectangle)
  ON_UPDATE_COMMAND_UI(ID_SCRIPT_OPTIONS_SELECTION_LINE, OnUpdateOptionsSelectionLine)

  ON_UPDATE_COMMAND_UI(ID_SCRIPT_CHECKSYNTAX,      OnUpdateScriptCheckSyntax)

  ON_UPDATE_COMMAND_UI(ID_SCRIPT_OPTIONS_TOGGLE_LINE_NUMBERS, OnUpdateOptionsToggleLineNumbers)
  ON_UPDATE_COMMAND_UI(ID_SCRIPT_OPTIONS_TOGGLE_COLORS, OnUpdateOptionsToggleColors)
  ON_UPDATE_COMMAND_UI(ID_SCRIPT_OPTIONS_SHOW_WHITESPACE, OnUpdateOptionsShowWhitespace)
  ON_UPDATE_COMMAND_UI(ID_SCRIPT_OPTIONS_WORD_WRAP, OnUpdateOptionsWordWrap)
  ON_UPDATE_COMMAND_UI(ID_SCRIPT_OPTIONS_TOGGLE_AUTOCOMPLETE, OnUpdateOptionsToggleAutoComplete)

  ON_COMMAND(ID_SCRIPT_TOOLS_SORT, OnScriptLineSorter)

  ON_NOTIFY(SCN_SAVEPOINTREACHED, ID_EDIT, OnSavePointReached)
  ON_NOTIFY(SCN_SAVEPOINTLEFT,    ID_EDIT, OnSavePointLeft)
  ON_NOTIFY(SCN_UPDATEUI,         ID_EDIT, OnPosChanged)
  ON_NOTIFY(SCN_CHARADDED,        ID_EDIT, OnCharAdded)

  ON_COMMAND(ID_FILE_ZOOM_IN, OnZoomIn)
  ON_COMMAND(ID_FILE_ZOOM_OUT, OnZoomOut)
  ON_COMMAND(ID_FILE_COPY,  OnCopy)
  ON_COMMAND(ID_FILE_PASTE, OnPaste)

  ON_REGISTERED_MESSAGE(s_FindReplaceMessage, OnFindReplace)

  ON_COMMAND(ID_SCRIPTVIEW_INSERT, OnScriptViewInsert)
  ON_COMMAND(ID_SCRIPTVIEW_DELETE, OnScriptViewDelete)

  ON_UPDATE_COMMAND_UI(ID_SCRIPTVIEW_INSERT, OnUpdateScriptViewInsert)
  ON_UPDATE_COMMAND_UI(ID_SCRIPTVIEW_DELETE, OnUpdateScriptViewDelete)

  ON_COMMAND(ID_SCRIPTVIEW_SPHEREFUNCTIONS,  OnScriptViewSphereFunctions)
  ON_COMMAND(ID_SCRIPTVIEW_CONTROLSTRUCTURES,  OnScriptViewControlStructures)
  ON_COMMAND(ID_SCRIPTVIEW_CURRENTSCRIPTFUNCTIONS, OnScriptViewCurrentScriptFunctions)
  ON_COMMAND(ID_SCRIPTVIEW_CLIPBOARDHISTORY, OnScriptViewClipboardHistory)
  ON_UPDATE_COMMAND_UI(ID_SCRIPTVIEW_SPHEREFUNCTIONS,  OnUpdateScriptViewSphereFunctions)
  ON_UPDATE_COMMAND_UI(ID_SCRIPTVIEW_CONTROLSTRUCTURES,  OnUpdateScriptViewControlStructures)
  ON_UPDATE_COMMAND_UI(ID_SCRIPTVIEW_CURRENTSCRIPTFUNCTIONS, OnUpdateScriptViewCurrentScriptFunctions)
  ON_UPDATE_COMMAND_UI(ID_SCRIPTVIEW_CLIPBOARDHISTORY, OnUpdateScriptViewClipboardHistory)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CScriptWindow::CScriptWindow(const char* filename, bool create_from_clipboard)
: CSaveableDocumentWindow(filename, IDR_SCRIPT)
, m_Created(false)
, m_SearchDialog(0)
, m_SyntaxHighlighted(true)
, m_ShowWhitespace(false)
, m_WordWrap(false)
, m_AllowAutoComplete(false)
, m_SelectionType(SC_SEL_STREAM)
, m_ListType(0)
{
  m_DocumentType = WA_SCRIPT;

  SetSaved(filename != NULL);
  SetModified(false);

  if (!Create()) {
    return;
  }

  if (!filename && create_from_clipboard) {
    OnPaste();
    SendEditor(SCI_SETSEL,  0, 0);
  }
  else
  if (filename) {
    LoadScript(filename);
  }

  //int HScrollWidth = 640; // scintilla's default scroll width of 2000 is too big
  /*
  int num_lines = SendEditor(SCI_GETLINECOUNT);
  for (int i = 0; i < num_lines; i++) {
    int linewidth =  SendEditor(SCI_LINELENGTH, i) * 8; // 8 = font size
    if (linewidth > HScrollWidth)
      HScrollWidth = linewidth;
  }
  */
  //SendEditor(SCI_SETSCROLLWIDTH, HScrollWidth);
}

////////////////////////////////////////////////////////////////////////////////

bool
CScriptWindow::Create()
{
  static HINSTANCE scintilla = NULL;
  if (!scintilla) {
    scintilla = LoadLibrary("SciLexer.dll");
    if (!scintilla) {
      ::MessageBox(
        NULL,
        "Could not load Scintilla component (SciLexer.dll)",
        "Script Editor",
        MB_OK | MB_ICONERROR);
      delete this;
      return false;
    }
  }
  

  // create the child window
  CSaveableDocumentWindow::Create(
    AfxRegisterWndClass(0, LoadCursor(NULL, IDC_ARROW), NULL, AfxGetApp()->LoadIcon(IDI_SCRIPT)));

  RECT Rect;
  GetClientRect(&Rect);

  int cx = Rect.right - Rect.left;
  int cy = Rect.bottom - Rect.top;
  int sidebar_width = 0;

  // creates the script view
  m_Editor = ::CreateWindow(
    "Scintilla",
    "Source",
    WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN,
    sidebar_width, 0,
    cx - sidebar_width, cy,
    m_hWnd,
    (HMENU)ID_EDIT,
    AfxGetApp()->m_hInstance,
    0);

  if (1) {
    // creates the list view
    m_List = ::CreateWindow(      
      "LISTBOX",
      "LISTBOX",
      WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN | WS_BORDER |
      LBS_EXTENDEDSEL | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT,
      0,
      0,
      sidebar_width,
      cy,
      m_hWnd,
      (HMENU)ID_EDIT,
      AfxGetApp()->m_hInstance,
      0);
  }

  Initialize();

  CreateList(m_ListType);
  
  ::ShowWindow(m_Editor, SW_SHOW);
  ::UpdateWindow(m_Editor);

  ::ShowWindow(m_List, SW_SHOW);
  if (::IsWindow(m_List))
    ::UpdateWindow(m_List);

  m_Created = true;

  // update the size of the view
  OnSize(0, Rect.right /*- Rect.left*/, Rect.bottom /*- Rect.top*/);

  // give the view focus
  ::SetFocus(m_Editor);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptWindow::CreateList(int type)
{
  m_ListType = type;

  if (!::IsWindow(m_List))
    return;

  ::SendMessage(m_List, LB_RESETCONTENT, 0, 0);

  if (m_ListType == 1) {
    unsigned int last_match = 0;
    unsigned int functions_length = strlen(sFunctionDefinitions);
    for (unsigned int i = 0; i < functions_length; i++) {
      if (sFunctionDefinitions[i] == ' ') {
        char buffer[4096] = {0};
        memcpy(buffer, sFunctionDefinitions + last_match, i - last_match);
        ::SendMessage(m_List, LB_ADDSTRING, 0, (LPARAM)buffer);
        last_match = i + 1;
      }
    }

    if (1) {
      char buffer[4096] = {0};
      memcpy(buffer, sFunctionDefinitions + last_match, strlen(sFunctionDefinitions) - last_match);
      ::SendMessage(m_List, LB_ADDSTRING, 0, (LPARAM)buffer);
    }
  }

  if (m_ListType == 2) {
    ::SendMessage(m_List, LB_ADDSTRING, 0, (LPARAM)"if (1)\n{\n  // ...\n}\n");
    ::SendMessage(m_List, LB_ADDSTRING, 0, (LPARAM)"if (1)\n{\n  // ...\n}\nelse\n{\n  // ...\n}\n");
    ::SendMessage(m_List, LB_ADDSTRING, 0, (LPARAM)"for (var i = 0; i < 10; i++)\n{\n  // ...\n}\n");
    ::SendMessage(m_List, LB_ADDSTRING, 0, (LPARAM)"while (1)\n{\n  // ...\n}\n");
    ::SendMessage(m_List, LB_ADDSTRING, 0, (LPARAM)"switch (1)\n{\n  case (1):\n    // ...\n  break;\n}\n");
    ::SendMessage(m_List, LB_ADDSTRING, 0, (LPARAM)"function func_name()\n{\n  // ...\n}\n");
  }

  if (m_ListType == 3) {
    ::SendMessage(m_List, LB_ADDSTRING, 0, (LPARAM)"todo todo...");
  }

  if (m_ListType == 4) {
    CMainWindow* main_window = GetMainWindow();
    if (main_window) {
      for (unsigned int i = 0; i < main_window->m_ClipboardHistory.size(); i++) {
        ::SendMessage(m_List, LB_ADDSTRING, 0, (LPARAM)main_window->m_ClipboardHistory[i].c_str());
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

CScriptWindow::ScriptType
CScriptWindow::GetScriptType()
{
  struct Local {
    static inline bool extension_compare(const char* path, const char* extension) {
      int path_length = strlen(path);
      int ext_length  = strlen(extension);
      return (
        path_length >= ext_length &&
        strcmp(path + path_length - ext_length, extension) == 0
      );
    }
  };

  if (Local::extension_compare(GetDocumentPath(), ".js")) {
    return SCRIPT_TYPE_JS;
  }

  if (Local::extension_compare(GetDocumentPath(), ".txt")) {
    return SCRIPT_TYPE_TXT;
  }

  return SCRIPT_TYPE_UNKNOWN;
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptWindow::SetScriptStyles()
{
  static const char key_words[] = 
    "break case catch continue default delete do else "
    "finally for function if in instanceof new return "
    "switch this throw try typeof var void while with "
    "true false undefined ";

  static const char reserved_words[] =
    "abstract enum int short "
    "boolean export interface static "
    "byte extends long super "
    "char final native synchronized "
    "class float package throws "
    "const goto private transient "
    "debugger implements protected volatile "
    "double import public ";

  static const COLORREF black   = 0x000000;
  static const COLORREF white   = 0xFFFFFF;
  static const COLORREF red     = RGB(0xFF, 0, 0);
  static const COLORREF green   = RGB(0, 0x80, 0);
  static const COLORREF blue    = RGB(0, 0, 0xFF);
  static const COLORREF purple  = RGB(0xFF, 0, 0xFF);
  static const COLORREF yellow  = RGB(0xFF, 0xFF, 0);
  static const COLORREF brown   = RGB(0xB5, 0x6F, 0x32);
  static const COLORREF darkred = RGB(0x80, 0, 0);

  if (GetScriptType() == SCRIPT_TYPE_UNKNOWN
   || GetScriptType() == SCRIPT_TYPE_JS)
  {
    SendEditor(SCI_SETLEXER, SCLEX_CPP);  // JavaScript uses the C++ lexer
    SendEditor(SCI_SETSTYLEBITS, 5);
    SendEditor(SCI_SETKEYWORDS, 0, (LPARAM)key_words);
    SendEditor(SCI_SETKEYWORDS, 1, (LPARAM)reserved_words);
  }
  else {
    SendEditor(SCI_SETLEXER, SCLEX_CONTAINER);
  }

  if (!(m_FontSize >= 0 && m_FontSize <= 72))
    m_FontSize = 10;

  SetStyle(STYLE_DEFAULT, black, white, m_FontSize, m_Fontface.c_str());
  SendEditor(SCI_STYLECLEARALL);

  // set all margins to zero width
  SendEditor(SCI_SETMARGINWIDTHN, 0, 0);
  SendEditor(SCI_SETMARGINWIDTHN, 1, 0);
  SendEditor(SCI_SETMARGINWIDTHN, 2, 0);

  // set all margin types
  SendEditor(SCI_SETMARGINTYPEN,  0, SC_MARGIN_NUMBER);
  SendEditor(SCI_SETMARGINWIDTHN, 1, SC_MARGIN_SYMBOL);
  SendEditor(SCI_SETMARGINWIDTHN, 2, SC_MARGIN_SYMBOL);

  // for code folding
  //SendEditor(SCI_SETPROPERTY, (WPARAM)"fold", (LPARAM)"1");
  //SendEditor(SCI_SETMARGINTYPEN, 1, SC_MARGIN_SYMBOL);
  //SendEditor(SCI_SETMARGINMASKN, 1, SC_MASK_FOLDERS);
  //SendEditor(SCI_SETMARGINWIDTHN, 1, 20);

  if (m_ShowLineNumbers) {  // resize the line number margin so we can see it
    int margin_size = SendEditor(SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM) "_99999");
    SendEditor(SCI_SETMARGINWIDTHN, 0, margin_size);
  }

  SendEditor(SCI_SETWRAPMODE, ((m_WordWrap) ? (SC_WRAP_WORD) : (SC_WRAP_NONE)));

  SetStyle(SCE_C_DEFAULT, black, white, m_FontSize, m_Fontface.c_str());

  if (m_TabWidth > 0) {
    SendEditor(SCI_SETTABWIDTH, m_TabWidth);
  }

  if (GetScriptType() == SCRIPT_TYPE_UNKNOWN
   || GetScriptType() == SCRIPT_TYPE_JS) {
    if (m_SyntaxHighlighted) {
      SetStyle(SCE_C_COMMENT,     green);
      SetStyle(SCE_C_COMMENTLINE, green);
      SetStyle(SCE_C_COMMENTDOC,  green);
      SetStyle(SCE_C_COMMENTLINEDOC, green);
      
      SetStyle(SCE_C_NUMBER,      darkred);
      SetStyle(SCE_C_WORD,        blue);

      if (m_KeyWordStyleIsBold) {
        SendEditor(SCI_STYLESETBOLD, SCE_C_WORD, 1);
      }

      SetStyle(SCE_C_STRING,      green);
      SetStyle(SCE_C_CHARACTER,   green);
      SetStyle(SCE_C_OPERATOR,    purple);
      SetStyle(SCE_C_IDENTIFIER,  black);
      SetStyle(SCE_C_WORD2,       red);

      SetStyle(STYLE_BRACELIGHT,  green);
      SetStyle(STYLE_BRACEBAD,    red);
    }
  }

  SendEditor(SCI_SETVIEWWS, ((m_ShowWhitespace) ? (SCWS_VISIBLEALWAYS) : (SCWS_INVISIBLE)));
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptWindow::Initialize()
{
  m_Fontface = Configuration::Get(KEY_SCRIPT_FONT_NAME);
  m_FontSize = Configuration::Get(KEY_SCRIPT_FONT_SIZE);
  m_SyntaxHighlighted = Configuration::Get(KEY_SCRIPT_SYNTAX_HIGHLIGHTED);
  m_TabWidth = Configuration::Get(KEY_SCRIPT_TAB_SIZE);
  m_KeyWordStyleIsBold = Configuration::Get(KEY_SCRIPT_KEYWORDS_IN_BOLD);
  m_ShowLineNumbers    = Configuration::Get(KEY_SCRIPT_SHOW_LINE_NUMBERS);
  m_ShowWhitespace     = Configuration::Get(KEY_SCRIPT_SHOW_WHITESPACE);
  m_WordWrap           = Configuration::Get(KEY_SCRIPT_WORD_WRAP);
  m_AllowAutoComplete  = Configuration::Get(KEY_SCRIPT_ALLOW_AUTOCOMPLETE);
  m_ListType           = Configuration::Get(KEY_SCRIPT_LIST_TYPE);
  SetScriptStyles();

  SetLineNumber(0);
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptWindow::SetStyle(
  int style,
  COLORREF fore,
  COLORREF back,
  int size,
  const char* face)
{
  SendEditor(SCI_STYLESETFORE, style, fore);
  SendEditor(SCI_STYLESETBACK, style, back);
  if (size >= 1) {
    SendEditor(SCI_STYLESETSIZE, style, size);
  }
  if (face) {
    SendEditor(SCI_STYLESETFONT, style, (LPARAM)face);
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg bool
CScriptWindow::LoadScript(const char* filename)
{
  // get the file size
  int file_size = FileSize(GetDocumentPath());
  if (file_size == -1)
    return false;

  // open the file
  FILE* file = fopen(filename, "rb");
  if (file == NULL)
    return false;

  /*
  // allocate a temporary storage buffer and read it
  char* buffer = new char[file_size + 1];
  if (buffer == NULL) return false;
  fread(buffer, sizeof(char), file_size, file);
  
  // null-terminate the string
  buffer[file_size] = 0;

  // put the buffer into the edit control
  SendEditor(SCI_SETSEL,  0, 0);
  SendEditor(SCI_SETTEXT, 0, (LPARAM)"");
  SendEditor(SCI_ADDTEXT, file_size, (LPARAM)buffer);

  // delete the buffer and close the file
  delete[] buffer;
  */

  int total_read = 0;
  char status_text[1024] = {0};
  sprintf (status_text, "%d%%...", (int)(((double)total_read / (double)file_size) * 100));
  GetStatusBar()->SetWindowText(status_text);

  while (!feof(file)) {
    char buffer[4096] = {0};
    int read_size = fread(buffer, sizeof(char), 4096, file);
    if (read_size > 0) {
      SendEditor(SCI_ADDTEXT, read_size, (LPARAM)buffer);
      total_read += read_size;
    }

    sprintf (status_text, "%d%%...", (int)(((double)total_read / (double)file_size) * 100));
    GetStatusBar()->SetWindowText(status_text);
  }

  sprintf (status_text, "%d%%...", (int)(((double)total_read / (double)file_size) * 100));
  GetStatusBar()->SetWindowText(status_text);
   
  fclose(file);

  SetModified(false);
  SendEditor(SCI_SETSAVEPOINT);
  SendEditor(SCI_EMPTYUNDOBUFFER);
  SendEditor(SCI_SETSEL,  0, 0);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptWindow::GetEditorText(CString& text)
{
  int length = SendEditor(SCI_GETLENGTH);
  char* str = new char[length + 1];
  if (str) {
    str[length] = 0;

    SendEditor(SCI_GETTEXT, length + 1, (LPARAM)str);
    text = str;
    delete[] str;
  }
}

////////////////////////////////////////////////////////////////////////////////

CString
CScriptWindow::GetSelection()
{
  int start = SendEditor(SCI_GETSELECTIONSTART);
  int end   = SendEditor(SCI_GETSELECTIONEND);
  if (end < start) {
    end = start;
  }

  int length = end - start;
  char* str = new char[length + 1];
  if (str) {
    str[length] = 0;
    SendEditor(SCI_GETSELTEXT, 0, (LPARAM)str);
    CString result(str);
    delete[] str;
    return result;
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnSize(UINT type, int cx, int cy)
{
  if (m_Created) {
    int sidebar_width = 0;

    if ( ::IsWindow(m_List) && ::IsWindowVisible(m_List) ) {
      RECT rect;
      if ( ::GetWindowRect(m_List, &rect) ) {
        sidebar_width = rect.right - rect.left;
      }
    }
 
    ::MoveWindow(m_Editor, sidebar_width, 0, cx - sidebar_width, cy, TRUE);
    if (::IsWindow(m_List))
      ::MoveWindow(m_List, 0, 0, sidebar_width, cy, TRUE);
  }

  CSaveableDocumentWindow::OnSize(type, cx, cy);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnSetFocus(CWnd* old)
{
  CSaveableDocumentWindow::OnSetFocus(old);
  if (m_Created) {
    ::SetFocus(m_Editor);
  }
}

////////////////////////////////////////////////////////////////////////////////

class ListBoxUtil
{
public:
  static int GetNumSelected(HWND list)
  {
    int i;
    int total = 0;  
    const int list_count = ::SendMessage(list, LB_GETCOUNT, 0, 0);
      
    for (i = 0; i < list_count; i++) {
      int selected = ::SendMessage(list, LB_GETSEL, (WPARAM) i, 0);
      if (selected > 0) {
        total += 1;
      }
    }

    return total;
  }

  static bool AreAnySelected(HWND list)
  {
    int i;
    const int list_count = ::SendMessage(list, LB_GETCOUNT, 0, 0);
      
    for (i = 0; i < list_count; i++) {
      int selected = ::SendMessage(list, LB_GETSEL, (WPARAM) i, 0);
      if (selected > 0) {
        return true;
      }
    }

    return false;
  }

  static void RemoveSelected(HWND list)
  {
    int i;
    const int list_count = ::SendMessage(list, LB_GETCOUNT, 0, 0);
    
    for (i = list_count - 1; i >= 0; i--) {
      int selected = ::SendMessage(list, LB_GETSEL, (WPARAM) i, 0);
      if (selected > 0) {
        ::SendMessage(list, LB_DELETESTRING, i, 0);
      }
    }
  }

  static void ProcessSelected(HWND list,
              void (*process_selected)(HWND list, int list_index, int process_index, void* data),
              void* data)
  {
    int i;
    int p;
    const int list_count = ::SendMessage(list, LB_GETCOUNT, 0, 0);
      
    for (i = 0, p = 0; i < list_count; i++)
    {
      int selected = ::SendMessage(list, LB_GETSEL, (WPARAM) i, 0);
      if (selected > 0) {
        process_selected(list, i, p, data);
        p++;
      }
    }
  }

};

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptViewInsert()
{
  struct Local {

    static void process_selected(HWND list, int list_index, int process_index, void* data)
    {
      HWND editor = (HWND) data;

      int pos  = ::SendMessage(editor, SCI_GETCURRENTPOS, 0, 0);
      int line = ::SendMessage(editor, SCI_LINEFROMPOSITION, pos, 0);

      std::string indent = "";

      if (line >= 0) {
        char text[1024 * 16];
        int line_length = ::SendMessage(editor, SCI_LINELENGTH, line, 0);
        if (line_length > 0 && line_length < sizeof(text)) {
          ::SendMessage(editor, SCI_GETLINE, line, (LRESULT)text);
          int i = 0;
          while (text[i] == ' ' || text[i] == '\t') {
            indent += text[i++];
          }
        }
      }

      int len = ::SendMessage(list, LB_GETTEXTLEN, list_index, 0);
      if (len > 0) {
        char* buffer = new char[len + 1];
        if (buffer) {
          ::SendMessage(list, LB_GETTEXT, list_index, (WPARAM)buffer);
          buffer[len] = 0;

          std::string str = "";

          if (process_index == 1) {
            str += "\n" + indent;
          }

          for (int i = 0; i < strlen(buffer); i++) {
            if (buffer[i] == '\n') {
              str += "\n";
              str += indent;
            }
            else {
              str += buffer[i];
            }
          }

          if (process_index > 0) {
            str += "\n" + indent;
          }

          delete[] buffer;

          if (process_index == 0) {
            ::SendMessage(editor, SCI_REPLACESEL, 0, (LRESULT)"");
          }
  
          ::SendMessage(editor, SCI_ADDTEXT, str.size(), (WPARAM)str.c_str());
        }
      }
    }

  };

  ListBoxUtil::ProcessSelected(m_List, Local::process_selected, m_Editor);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptViewDelete()
{
  ListBoxUtil::RemoveSelected(m_List);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnUpdateScriptViewInsert(CCmdUI* cmdui)
{
  cmdui->Enable((ListBoxUtil::GetNumSelected(m_List) > 0 && ListBoxUtil::GetNumSelected(m_List) < 10) ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnUpdateScriptViewDelete(CCmdUI* cmdui)
{
  cmdui->Enable(ListBoxUtil::AreAnySelected(m_List) ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptViewSphereFunctions()
{
  CreateList(1);
  RememberConfiguration();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptViewControlStructures()
{
  CreateList(2);
  RememberConfiguration();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptViewCurrentScriptFunctions()
{
  CreateList(3);
  RememberConfiguration();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptViewClipboardHistory()
{
  CreateList(4);
  RememberConfiguration();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnUpdateScriptViewSphereFunctions(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_ListType == 1 ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnUpdateScriptViewControlStructures(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_ListType == 2 ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnUpdateScriptViewCurrentScriptFunctions(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_ListType == 3 ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnUpdateScriptViewClipboardHistory(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_ListType == 4 ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnContextMenu(CWnd* pWnd, CPoint point)
{
  HMENU _menu = LoadMenu(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_SCRIPTVIEW));
  HMENU menu = GetSubMenu(_menu, 0);

  TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, m_hWnd, NULL);

  DestroyMenu(_menu);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptCheckSyntax()
{
  CString text;
  GetEditorText(text);

  // verify the script
  sCompileError error;
  if (!VerifyScript(text, error))
  {
    // show the error message
    MessageBox(error.m_Message.c_str(), "Check syntax", MB_OK);
    if (error.m_Token.length() > 0)
      SendEditor(
        SCI_SETSEL,
        error.m_TokenStart,
        error.m_TokenStart + error.m_Token.length());
  }
  else
    MessageBox("Script is valid");
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptFind()
{
  if (!m_SearchDialog) {
    m_SearchDialog = new CFindReplaceDialog;
    if (m_SearchDialog) {
      
      SendEditor(SCI_SETSELECTIONMODE, SC_SEL_STREAM);
      CString text = GetSelection();
      SendEditor(SCI_SETSELECTIONMODE, m_SelectionType);

      m_SearchDialog->Create(true, text, NULL, FR_DOWN, this);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptReplace()
{
  if (!m_SearchDialog) {
    m_SearchDialog = new CFindReplaceDialog;
    if (m_SearchDialog) {

      SendEditor(SCI_SETSELECTIONMODE, SC_SEL_STREAM);
      CString text = GetSelection();
      SendEditor(SCI_SETSELECTIONMODE, m_SelectionType);

      m_SearchDialog->Create(false, text, NULL, FR_DOWN, this);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptGotoLine()
{
  int pos = SendEditor(SCI_GETCURRENTPOS);
  int cur_line = SendEditor(SCI_LINEFROMPOSITION, pos) + 1;
  int max_line = SendEditor(SCI_GETLINECOUNT);

  char string[1024];
  sprintf(string, "Line Number [%d / %d]", 1, max_line);

  CNumberDialog dialog("Goto Line", string, cur_line, 1, max_line);
  if (dialog.DoModal()) {
    if (dialog.GetValue() != cur_line) {
      SendEditor(SCI_GOTOLINE, dialog.GetValue() - 1);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnSavePointReached(NMHDR* nmhdr, LRESULT* result)
{
  SetModified(false);
  SetScriptStyles();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnSavePointLeft(NMHDR* nmhdr, LRESULT* result)
{
  SetModified(true);
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptWindow::OnToolChanged(UINT id, int tool_index) {

}

BOOL
CScriptWindow::IsToolAvailable(UINT id)
{
  BOOL available = FALSE;
  
  switch (id) {
    case ID_FILE_COPY:
      if (SendEditor(SCI_GETSELECTIONSTART) - SendEditor(SCI_GETSELECTIONEND)) available = TRUE;
    break;
    case ID_FILE_PASTE:
      if (SendEditor(SCI_CANPASTE) != 0) available = TRUE;
    break;
  }

  return available;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnCopy()
{
  SendEditor(SCI_COPY);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnPaste()
{
  SendEditor(SCI_PASTE);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnZoomIn()
{
  SendEditor(SCI_ZOOMIN);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnZoomOut()
{
  SendEditor(SCI_ZOOMOUT);
}

////////////////////////////////////////////////////////////////////////////////

static
bool is_brace(char ch) {
  return (ch == '{' || ch == '[' || ch == '('
       || ch == '}' || ch == ']' || ch == ')');
}

void
CScriptWindow::UpdateBraceHighlight()
{
  int pos = SendEditor(SCI_GETCURRENTPOS);
  if (is_brace(SendEditor(SCI_GETCHARAT, pos)))
  {
    int m = SendEditor(SCI_BRACEMATCH, pos, 0);
    if (m != -1) {
      SendEditor(SCI_BRACEHIGHLIGHT, pos, m);
    } 
    else {
      SendEditor(SCI_BRACEBADLIGHT, pos);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnPosChanged(NMHDR* nmhdr, LRESULT* result) {
  SCNotification* notify = (SCNotification*)nmhdr;
  int pos = SendEditor(SCI_GETCURRENTPOS);
  int line = SendEditor(SCI_LINEFROMPOSITION, pos);
  SetLineNumber(line);
  if (m_SyntaxHighlighted)
    if (GetScriptType() == SCRIPT_TYPE_UNKNOWN || GetScriptType() == SCRIPT_TYPE_JS)
      UpdateBraceHighlight();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnCharAdded(NMHDR* nmhdr, LRESULT* result) {

  SCNotification* notify = (SCNotification*)nmhdr;

#if 1

  if (m_AllowAutoComplete && notify->ch != '\r' && notify->ch != '\n' && notify->ch != ' ') {
    if (GetScriptType() == SCRIPT_TYPE_UNKNOWN || GetScriptType() == SCRIPT_TYPE_JS)
    {
      SendEditor(SCI_SETWORDCHARS, 0, (LPARAM)"_.abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
      int cur_pos  = SendEditor(SCI_GETCURRENTPOS);
      int word_pos = SendEditor(SCI_WORDSTARTPOSITION, cur_pos);
      int len = cur_pos - word_pos;
      if (word_pos >= 0 && len > 0) {
        SendEditor(SCI_AUTOCSHOW, cur_pos - word_pos, (LPARAM)sFunctionDefinitions);
      }

      SendEditor(SCI_SETCHARSDEFAULT);
    }
  }

#endif

  if (notify->ch == '\n') {
    int pos  = SendEditor(SCI_GETCURRENTPOS);
    int line = SendEditor(SCI_LINEFROMPOSITION, pos);

    if (line > 0) {
      char text[1024 * 16];
      int line_length = SendEditor(SCI_LINELENGTH, line - 1);
      if (line_length > 0 && line_length < sizeof(text)) {
        SendEditor(SCI_GETLINE, line - 1, (LRESULT)text);
        std::string str;
        int i = 0;
        while (text[i] == ' ' || text[i] == '\t') {
          str += text[i++];
        }

        /*
        if (1) {
          i = line_length;
          while (text[i] != '{' && i >= 0 && i > line_length - 5) { i--; }
          if (text[i] == '{') {
            str += "  ";
          }

          i = 0;
          while (i++ < 100) {
            int m = SendEditor(SCI_BRACEMATCH, i, 0);
            if (m != -1) {
              pos = pos;
            }
          }

          if (1) {

            while (text[i] != '}' && i >= 0 && i > line_length - 5) { i--; }
            if (text[i] == '}') {
              str.erase(0, 2);
            }
          }
        }
        */

        SendEditor(SCI_REPLACESEL, 0, (LRESULT)str.c_str());
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptWindow::SetLineNumber(int line) {
  char str[80];
  int pos = SendEditor(SCI_GETCURRENTPOS) - SendEditor(SCI_POSITIONFROMLINE, line);
  line += 1;
  sprintf(str, "Line: %d Position: %d", line, pos);
  GetStatusBar()->SetPaneText(1, str);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg LRESULT
CScriptWindow::OnFindReplace(WPARAM, LPARAM)
{
  CString str;
  GetEditorText(str);

  if (m_SearchDialog->IsTerminating()) {
    m_SearchDialog = NULL;
  } else if (m_SearchDialog->FindNext()) {

    TextToFind ttf;
    ttf.chrg.cpMin = SendEditor(SCI_GETSELECTIONSTART) + 1;
    ttf.chrg.cpMax = str.GetLength();
    CString find_string(m_SearchDialog->GetFindString());
    ttf.lpstrText = find_string.GetBuffer(0);
    
    int options = 0;
    options |= m_SearchDialog->MatchCase() ? SCFIND_MATCHCASE : 0;
    options |= m_SearchDialog->MatchWholeWord() ? SCFIND_WHOLEWORD : 0;
    BOOL search_down = m_SearchDialog->SearchDown();

    if (!search_down) {
      ttf.chrg.cpMin = ttf.chrg.cpMin - 2; // selection_start - 1
      ttf.chrg.cpMax = 0;
    }

    if (SendEditor(SCI_FINDTEXT, options, (LPARAM)&ttf) == -1) {
      m_SearchDialog->MessageBox("No more matches!");
    } else {
      SendEditor(SCI_SETSEL, ttf.chrgText.cpMin, ttf.chrgText.cpMax);
    }
    
  } else if (m_SearchDialog->ReplaceCurrent()) {

    // get currently selected text
    CString selection(GetSelection());
    CString find_string(m_SearchDialog->GetFindString());

    // build the data structure we need to find the next text
    // before we replace the string
    TextToFind ttf;
    ttf.chrg.cpMin = SendEditor(SCI_GETSELECTIONSTART);
    ttf.chrg.cpMax = str.GetLength();
    ttf.lpstrText = find_string.GetBuffer(0);
    int options = 0;  // ?

    // if the selection is what we want to replace, then do so
    CString replace_string(m_SearchDialog->GetReplaceString());
    if (selection == find_string) {

      // actually do the string replacing
      SendEditor(SCI_REPLACESEL, 0, (LPARAM)(const char*)replace_string);

      // now try to find the next one
      ttf.chrg.cpMin += strlen(replace_string);
      if (SendEditor(SCI_FINDTEXT, options, (LPARAM)&ttf) == -1) {
        m_SearchDialog->MessageBox("No more matches!");
      } else {
        SendEditor(SCI_SETSEL, ttf.chrgText.cpMin, ttf.chrgText.cpMax);
      }
    } else {

      if (SendEditor(SCI_FINDTEXT, options, (LPARAM)&ttf) == -1) {
        m_SearchDialog->MessageBox("No matches!");
      } else {
        SendEditor(SCI_SETSEL, ttf.chrgText.cpMin, ttf.chrgText.cpMax);
      }
    }

  } else if (m_SearchDialog->ReplaceAll()) {

    // get currently selected text
    CString selection(GetSelection());
    CString find_string(m_SearchDialog->GetFindString());
    CString replace_string(m_SearchDialog->GetReplaceString());

    // build the data structure we need to find the next text
    // before we replace the string
    TextToFind ttf;
    ttf.chrg.cpMin = SendEditor(SCI_GETSELECTIONSTART);
    ttf.chrg.cpMax = str.GetLength();
    ttf.lpstrText = find_string.GetBuffer(0);
    int options = 0;  // ?

    if (selection != find_string) {
      if (SendEditor(SCI_FINDTEXT, options, (LPARAM)&ttf) != -1) {
        SendEditor(SCI_SETSEL, ttf.chrgText.cpMin, ttf.chrgText.cpMax);
      }
    }

    // if the selection is what we want to replace, then do so
    while (GetSelection() == find_string) {

      // actually do the string replacing
      SendEditor(SCI_REPLACESEL, 0, (LPARAM)(const char*)replace_string);

      // now try to find the next one
      ttf.chrg.cpMin += strlen(replace_string);
      if (SendEditor(SCI_FINDTEXT, options, (LPARAM)&ttf) != -1) {
        SendEditor(SCI_SETSEL, ttf.chrgText.cpMin, ttf.chrgText.cpMax);
      }
    }
    
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

bool
CScriptWindow::GetSavePath(char* path)
{
  CScriptFileDialog Dialog(FDM_SAVE);
  if (Dialog.DoModal() != IDOK)
    return false;

  strcpy(path, Dialog.GetPathName());
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CScriptWindow::SaveDocument(const char* path)
{
  FILE* file = fopen(path, "wb");
  if (file == NULL)
    return false;

  bool saved = false;

  int length = SendEditor(SCI_GETLENGTH);
  char* str = new char[length + 1];
  if (str) {
    str[length] = 0;
    SendEditor(SCI_GETTEXT, length + 1, (LPARAM)str);

    fwrite(str, 1, length, file);
    fclose(file);

    delete[] str;
    saved = true;

    SendEditor(SCI_SETSAVEPOINT);
  }

  return saved;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnScriptLineSorter()
{
  CLineSorterDialog dialog;
  if (dialog.DoModal() != IDOK) {
    return;
  }

  CLineSorter line_sorter;
  line_sorter.m_Editor = m_Editor;

  line_sorter.delete_duplicates = dialog.ShouldRemoveDuplicateLines();
  line_sorter.sort_lines        = dialog.ShouldSortLines();
  line_sorter.reverse_lines     = dialog.ShouldReverseLines();
  line_sorter.ignore_case       = dialog.ShouldIgnoreCase();

  line_sorter.Sort();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnOptionsToggleColors()
{
  m_SyntaxHighlighted = !m_SyntaxHighlighted;
  SetScriptStyles();
  RememberConfiguration();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnOptionsSetScriptFont()
{
  // Show the font dialog with all the default settings.
  LOGFONT lf;
  memset((void*)&lf, 0, sizeof(lf));
  if (m_Fontface.size() < 31) {
    memcpy(lf.lfFaceName, m_Fontface.c_str(), m_Fontface.size());
  }

  CFontDialog dlg(&lf);
  if (dlg.DoModal() == IDOK) {

    m_Fontface = std::string(dlg.GetFaceName());
    m_FontSize = dlg.GetSize() / 10; // for a 12pt font dlg.GetSize() is 120
    SetScriptStyles();
    RememberConfiguration();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnOptionsSetTabSize()
{
  CNumberDialog dialog("Enter new tab width value", "Value", m_TabWidth, 2, 8);
  if (dialog.DoModal() == IDOK) {
    m_TabWidth = dialog.GetValue();
    SetScriptStyles();
    RememberConfiguration();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnOptionsToggleLineNumbers()
{
  m_ShowLineNumbers = !m_ShowLineNumbers;
  SetScriptStyles();
  RememberConfiguration();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnOptionsShowWhitespace()
{
  m_ShowWhitespace = !m_ShowWhitespace;
  SetScriptStyles();
  RememberConfiguration();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnOptionsWordWrap()
{
  m_WordWrap = !m_WordWrap;
  SetScriptStyles();
  RememberConfiguration();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnOptionsSelectionStream()
{
  m_SelectionType = SC_SEL_STREAM;
  SendEditor(SCI_SETSELECTIONMODE, m_SelectionType);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnOptionsSelectionRectangle()
{
  m_SelectionType = SC_SEL_RECTANGLE;
  SendEditor(SCI_SETSELECTIONMODE, m_SelectionType);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnOptionsSelectionLine()
{
  m_SelectionType = SC_SEL_LINES;
  SendEditor(SCI_SETSELECTIONMODE, m_SelectionType);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnUpdateOptionsSelectionStream(CCmdUI* cmdui)
{
  cmdui->SetCheck((SendEditor(SCI_GETSELECTIONMODE) == SC_SEL_STREAM) ? TRUE : FALSE);
}

afx_msg void
CScriptWindow::OnUpdateOptionsSelectionRectangle(CCmdUI* cmdui)
{
  cmdui->SetCheck((SendEditor(SCI_GETSELECTIONMODE) == SC_SEL_RECTANGLE) ? TRUE : FALSE);
}

afx_msg void
CScriptWindow::OnUpdateOptionsSelectionLine(CCmdUI* cmdui)
{
  cmdui->SetCheck((SendEditor(SCI_GETSELECTIONMODE) == SC_SEL_LINES) ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnUpdateScriptCheckSyntax(CCmdUI* cmdui)
{
  if (!(GetScriptType() == SCRIPT_TYPE_UNKNOWN
    || GetScriptType() == SCRIPT_TYPE_JS)) {
    cmdui->Enable(FALSE);
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnUpdateOptionsToggleLineNumbers(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_ShowLineNumbers ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnUpdateOptionsToggleColors(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_SyntaxHighlighted ? TRUE : FALSE);
  if (!(GetScriptType() == SCRIPT_TYPE_UNKNOWN || GetScriptType() == SCRIPT_TYPE_JS)) {
    cmdui->Enable(FALSE);
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnUpdateOptionsShowWhitespace(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_ShowWhitespace ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnUpdateOptionsWordWrap(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_WordWrap ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnOptionsToggleAutoComplete()
{
  m_AllowAutoComplete = !m_AllowAutoComplete;
  RememberConfiguration();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CScriptWindow::OnUpdateOptionsToggleAutoComplete(CCmdUI* cmdui)
{
  cmdui->SetCheck(m_AllowAutoComplete ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptWindow::RememberConfiguration()
{
  Configuration::Set(KEY_SCRIPT_FONT_NAME, m_Fontface);
  Configuration::Set(KEY_SCRIPT_FONT_SIZE, m_FontSize);
  Configuration::Set(KEY_SCRIPT_TAB_SIZE, m_TabWidth);
  Configuration::Set(KEY_SCRIPT_SYNTAX_HIGHLIGHTED, m_SyntaxHighlighted);
  Configuration::Set(KEY_SCRIPT_SHOW_LINE_NUMBERS, m_ShowLineNumbers);
  Configuration::Set(KEY_SCRIPT_SHOW_WHITESPACE, m_ShowWhitespace);
  Configuration::Set(KEY_SCRIPT_WORD_WRAP, m_WordWrap);
  Configuration::Set(KEY_SCRIPT_ALLOW_AUTOCOMPLETE, m_AllowAutoComplete);
  Configuration::Set(KEY_SCRIPT_LIST_TYPE, m_ListType);
}

////////////////////////////////////////////////////////////////////////////////
