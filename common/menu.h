#ifndef __XMENU_H__
#define __XMENU_H__


#include "begin_c_prototypes.h"

  extern void SetMenuCheck(HMENU menu, UINT item, BOOL checked);
  extern BOOL GetMenuCheck(HMENU menu, UINT item);

  extern void EnableItem(HMENU menu, UINT item);
  extern void DisableItem(HMENU menu, UINT item);

#include "end_c_prototypes.h"


#endif
