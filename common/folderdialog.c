#include <shlobj.h>
#include "types.h"
#include "folderdialog.h"


////////////////////////////////////////////////////////////////////////////////

bool BrowseForFolderDialog(HWND parent, const char* title, char* folder)
{
  BROWSEINFO bi;
  LPITEMIDLIST idlist;
  LPMALLOC malloc;

  bi.hwndOwner      = parent;
  bi.pidlRoot       = NULL;
  bi.pszDisplayName = NULL;
  bi.lpszTitle      = title;
  bi.ulFlags        = BIF_RETURNONLYFSDIRS;
  bi.lpfn           = NULL;
  bi.lParam         = 0;
  bi.iImage         = 0;

  // browse dialog
  idlist = SHBrowseForFolder(&bi);
  if (idlist == NULL)
    return false;

  // convert id list into path
  if (SHGetPathFromIDList(idlist, folder) == FALSE)
    return false;

  // free the id list
  SHGetMalloc(&malloc);
  malloc->lpVtbl->Free(malloc, idlist);
  malloc->lpVtbl->Release(malloc);

  return true;
}

////////////////////////////////////////////////////////////////////////////////
