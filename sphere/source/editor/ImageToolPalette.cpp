#include "ImageToolPalette.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include "resource.h"


////////////////////////////////////////////////////////////////////////////////

CImageToolPalette::CImageToolPalette(CDocumentWindow* owner, IToolPaletteHandler* handler)
: CToolPalette(owner, handler, "Image Tools",
  Configuration::Get(KEY_IMAGE_TOOLS_RECT),
  Configuration::Get(KEY_IMAGE_TOOLS_VISIBLE))
{
  AddTool(IDI_IMAGETOOL_PENCIL);
  AddTool(IDI_IMAGETOOL_LINE);
  AddTool(IDI_IMAGETOOL_RECTANGLE);
  AddTool(IDI_IMAGETOOL_CIRCLE);
  AddTool(IDI_IMAGETOOL_FILL);
}

////////////////////////////////////////////////////////////////////////////////

void
CImageToolPalette::Destroy()
{
  // store state
  RECT rect;
  GetWindowRect(&rect);
  Configuration::Set(KEY_IMAGE_TOOLS_RECT, rect);

  Configuration::Set(KEY_IMAGE_TOOLS_VISIBLE, IsWindowVisible() != FALSE);

  CToolPalette::Destroy();
}

////////////////////////////////////////////////////////////////////////////////
