#include "MapToolPalette.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include "resource.h"


////////////////////////////////////////////////////////////////////////////////

CMapToolPalette::CMapToolPalette(CDocumentWindow* owner, IToolPaletteHandler* handler)
: CToolPalette(owner, handler, "Map Tools",
  Configuration::Get(KEY_MAP_TOOLS_RECT),
  Configuration::Get(KEY_MAP_TOOLS_VISIBLE))
{
  AddTool(IDI_MAPTOOL_1X1,         "1x1");
  AddTool(IDI_MAPTOOL_3X3,         "3x3");
  AddTool(IDI_MAPTOOL_5X5,         "5x5");
  AddTool(IDI_MAPTOOL_SELECTTILE,  "Select Tile");
  AddTool(IDI_MAPTOOL_FILLAREA,    "Fill Area");
  AddTool(IDI_MAPTOOL_COPYAREA,    "Copy Area");
  AddTool(IDI_MAPTOOL_PASTE,       "Paste");
  AddTool(IDI_MAPTOOL_COPYENTITY,  "Copy Entity");
  AddTool(IDI_MAPTOOL_PASTEENTITY, "Paste Entity");
  AddTool(IDI_MAPTOOL_OBS_SEGMENT, "Obstruction Segment");
  AddTool(IDI_MAPTOOL_OBS_DELETE,  "Delete Obstruction Segment");
  AddTool(IDI_MAPTOOL_OBS_MOVE_PT, "Move Obstruction Segment Point");
  AddTool(IDI_MAPTOOL_ZONEADD,     "Add a Zone");
  AddTool(IDI_MAPTOOL_ZONEEDIT,    "Move a Zone");
  AddTool(IDI_MAPTOOL_ZONEDELETE,  "Delete a Zone");
  AddTool(IDI_MAPTOOL_MULTI_TILE,  "Place Multiple Tiles");
}

////////////////////////////////////////////////////////////////////////////////

void
CMapToolPalette::Destroy()
{
  // store state
  RECT rect;
  GetWindowRect(&rect);
  Configuration::Set(KEY_MAP_TOOLS_RECT, rect);

  Configuration::Set(KEY_MAP_TOOLS_VISIBLE, IsWindowVisible() != FALSE);

  CToolPalette::Destroy();
}

////////////////////////////////////////////////////////////////////////////////
