#ifndef IMAGE_TOOL_PALETTE_HPP
#define IMAGE_TOOL_PALETTE_HPP


#include "ToolPalette.hpp"


class wImageToolPalette : public wToolPalette
{
public:
  wImageToolPalette(wDocumentWindow* owner, IToolPaletteHandler* handler);

  virtual bool Destroy();
};


#endif
