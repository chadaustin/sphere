#include "ImageToolPalette.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"

#include "icons/pencil.xpm"
#include "icons/line.xpm"
#include "icons/rectangle.xpm"
#include "icons/circle.xpm"
#include "icons/bucket.xpm"

////////////////////////////////////////////////////////////////////////////////

wImageToolPalette::wImageToolPalette(wDocumentWindow* owner, IToolPaletteHandler* handler)
: wToolPalette(owner, handler, "Image Tools",
  Configuration::Get(KEY_IMAGE_TOOLS_RECT()),
  Configuration::Get(KEY_IMAGE_TOOLS_VISIBLE()))
{
  AddTool((const char **)pencil_xpm,    "Pencil");
  AddTool((const char **)line_xpm,      "Line");
  AddTool((const char **)rectangle_xpm, "Rectangle");
  AddTool((const char **)circle_xpm,    "Circle");
  AddTool((const char **)bucket_xpm,    "Fill");
}

////////////////////////////////////////////////////////////////////////////////

bool
wImageToolPalette::Destroy()
{
  // store state
  wxRect rect = GetRect();
  Configuration::Set(KEY_IMAGE_TOOLS_RECT(), rect);

  Configuration::Set(KEY_IMAGE_TOOLS_VISIBLE(), true); /*todo:IsWindowVisible() != FALSE);*/

  return wToolPalette::Destroy();
}

////////////////////////////////////////////////////////////////////////////////
