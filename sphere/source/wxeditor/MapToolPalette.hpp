#ifndef MAP_TOOL_PALETTE_HPP
#define MAP_TOOL_PALETTE_HPP


#include "ToolPalette.hpp"


class wMapToolPalette : public wToolPalette
{
public:
  wMapToolPalette(wDocumentWindow* owner, IToolPaletteHandler* handler);

  virtual bool Destroy();
};


#endif
