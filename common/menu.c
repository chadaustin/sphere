#include <windows.h>
#include "menu.h"


////////////////////////////////////////////////////////////////////////////////

void SetMenuCheck(HMENU menu, UINT item, BOOL checked)
{
  MENUITEMINFO mii;

  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_STATE;
  mii.fState = (checked ? MFS_CHECKED : MFS_UNCHECKED);

  SetMenuItemInfo(menu, item, FALSE, &mii);
}

////////////////////////////////////////////////////////////////////////////////

BOOL GetMenuCheck(HMENU menu, UINT item)
{
  MENUITEMINFO mii;

  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_STATE;

  GetMenuItemInfo(menu, item, FALSE, &mii);

  if (mii.fState == MFS_CHECKED)
    return TRUE;
  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

void EnableItem(HMENU menu, UINT item)
{
  MENUITEMINFO mii;
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_STATE;
  mii.fState = MFS_ENABLED;
  SetMenuItemInfo(menu, item, FALSE, &mii);
}

////////////////////////////////////////////////////////////////////////////////

void DisableItem(HMENU menu, UINT item)
{
  MENUITEMINFO mii;
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_STATE;
  mii.fState = MFS_DISABLED;
  SetMenuItemInfo(menu, item, FALSE, &mii);
}

////////////////////////////////////////////////////////////////////////////////
