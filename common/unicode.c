#include <stdlib.h>
#include <string.h>
#include "../common/unicode.h"



int atow(wchar* dest, const achar* src, int length)
{
  return mbstowcs(dest, src, length);
}


int wtoa(achar* dest, const wchar* src, int length)
{
  return wcstombs(dest, src, length);
}


int atoa(achar* dest, const achar* src, int length)
{
  return strlen(strncpy(dest, src, length));
}


int wtow(wchar* dest, const wchar* src, int length)
{
  return wcslen(wcsncpy(dest, src, length));
}
