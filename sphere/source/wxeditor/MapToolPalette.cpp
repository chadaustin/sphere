#include "MapToolPalette.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
//#include "resource.h"

#include "icons/1x1.xpm"
#include "icons/3x3.xpm"
#include "icons/5x5.xpm"
#include "icons/areacopy.xpm"
#include "icons/areapaste.xpm"
#include "icons/circle.xpm"
#include "icons/entitycopy.xpm"
#include "icons/entitypaste.xpm"
#include "icons/picker.xpm"
#include "icons/rectfill.xpm"
#include "icons/obsadd.xpm"
#include "icons/obsdel.xpm"
#include "icons/obsmove.xpm"
#include "icons/zoneadd.xpm"
#include "icons/zonedel.xpm"
#include "icons/zonemove.xpm"


////////////////////////////////////////////////////////////////////////////////

wMapToolPalette::wMapToolPalette(wDocumentWindow* owner, IToolPaletteHandler* handler)
: wToolPalette(owner, handler, "Map Tools",
  Configuration::Get(KEY_MAP_TOOLS_RECT()),
  Configuration::Get(KEY_MAP_TOOLS_VISIBLE()))
{
  AddTool((const char **)p1x1_xpm,         "1x1");
  AddTool((const char **)p3x3_xpm,         "3x3");
  AddTool((const char **)p5x5_xpm,         "5x5");
  AddTool((const char **)picker_xpm,       "Select Tile");
  AddTool((const char **)rectfill_xpm,     "Fill Area");
  AddTool((const char **)areacopy_xpm,     "Copy Area");
  AddTool((const char **)areapaste_xpm,    "Paste");
  AddTool((const char **)entitycopy_xpm,   "Copy Entity");
  AddTool((const char **)entitypaste_xpm,  "Paste Entity");
  AddTool((const char **)obsadd_xpm,       "Obstruction Segment");
  AddTool((const char **)obsdel_xpm,       "Delete Obstruction Segment");
  AddTool((const char **)obsmove_xpm,      "Move Obstruction Segment Point");
  AddTool((const char **)zoneadd_xpm,      "Add a Zone");
  AddTool((const char **)zonemove_xpm,     "Move a Zone");
  AddTool((const char **)zonedel_xpm,      "Delete a Zone");
}

////////////////////////////////////////////////////////////////////////////////

bool
wMapToolPalette::Destroy()
{
  // store state
  wxRect rect = GetRect();
  Configuration::Set(KEY_MAP_TOOLS_RECT(), rect);

  Configuration::Set(KEY_MAP_TOOLS_VISIBLE(), true); /*todo:IsWindowVisible() != FALSE);*/

  return wToolPalette::Destroy();
}

////////////////////////////////////////////////////////////////////////////////
