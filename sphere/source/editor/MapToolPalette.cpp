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
  AddTool(IDI_MAPTOOL_1X1);
  AddTool(IDI_MAPTOOL_3X3);
  AddTool(IDI_MAPTOOL_5X5);
  AddTool(IDI_MAPTOOL_SELECTTILE);
  AddTool(IDI_MAPTOOL_FILLAREA);
  AddTool(IDI_MAPTOOL_COPYAREA);
  AddTool(IDI_MAPTOOL_PASTE);
  AddTool(IDI_MAPTOOL_COPYENTITY);
  AddTool(IDI_MAPTOOL_PASTEENTITY);
  AddTool(IDI_MAPTOOL_OBS_SEGMENT);
  AddTool(IDI_MAPTOOL_OBS_DELETE);
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
