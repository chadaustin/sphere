#include <stdio.h>
#include "win32x.hpp"


////////////////////////////////////////////////////////////////////////////////

BOOL WritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nInt, LPCTSTR lpFileName)
{
  char str[80];
  sprintf(str, "%d", nInt);
  return WritePrivateProfileString(lpAppName, lpKeyName, str, lpFileName);
}

////////////////////////////////////////////////////////////////////////////////
