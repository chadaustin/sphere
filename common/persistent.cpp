#include <windows.h>
#include <stdlib.h>
#include "persistent.hpp"
#include "../common/unicode.h"
#include "../common/resize.hpp"


static const int    MaxKeyLength  = 32;
static const int    MaxDataLength = 256;


struct SPersistentItem
{
  char key[MaxKeyLength];
  byte data[MaxDataLength];
};


static const tchar* const PersistFile   = ttext("config");

static int              ItemCount = 0;
static SPersistentItem* Items     = NULL;


bool
_PERSISTENT::Load(const char* key, int* itemnumber, int datalength)
{
  static bool Loaded = false; // cdr - this doesn't need to be visable to entire file so I moved it
  
  if (datalength > MaxDataLength) {
    // not in CE:  throw "Persistent object created with type larger than max allowed size (256 bytes)";
    return false; 
  }

  // make sure the data file has been loaded
  if (!Loaded)
  {
    // open persistent file
    HANDLE file = CreateFile(PersistFile,
                             GENERIC_READ,
                             0,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_HIDDEN,
                             NULL);

    // if the file doesn't exist, there are no items
    if (file == INVALID_HANDLE_VALUE)
    {
      ItemCount = 0;
      Items     = NULL;
    }
    else  // load the items
    {
      DWORD bytes_read;
      ReadFile(file, &ItemCount, sizeof(ItemCount), &bytes_read, NULL);
      Items = new SPersistentItem[ItemCount];

      // read each item
      for (int i = 0; i < ItemCount; i++)
        ReadFile(file, Items + i, sizeof(SPersistentItem), &bytes_read, NULL);

      CloseHandle(file);
    }
    Loaded = true;
  }


  // find item in list
  for (int i = 0; i < ItemCount; i++)
  {
    // found it!
    if (strncmp(key, Items[i].key, MaxKeyLength) == 0)
    {
      *itemnumber = i;
      return true;
    }
  }


  // we couldn't find one, so add it to the list
  resize(Items, ItemCount, ItemCount + 1);
  strncpy(Items[ItemCount].key, key, MaxKeyLength);
  memset(Items[ItemCount].data, 0, MaxDataLength);

  *itemnumber = ItemCount;

  ItemCount++;

  // save it in case two persistent objects are created (and not modified) and the
  // item count is updated, but the items aren't serialized
  Save(*itemnumber);

  return false;

}




void
_PERSISTENT::Save(int itemnumber)
{
  // open the file
  HANDLE file = CreateFile(PersistFile,
                           GENERIC_WRITE,
                           0,
                           NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_HIDDEN,
                           NULL);
  if (file == INVALID_HANDLE_VALUE)
    return;

  // update the item count
  DWORD bytes_written;
  WriteFile(file, &ItemCount, sizeof(ItemCount), &bytes_written, NULL);

  // write the new item
  SetFilePointer(file,
                 sizeof(ItemCount) + itemnumber * sizeof(SPersistentItem),
                 NULL,
                 FILE_BEGIN);
  WriteFile(file, Items + itemnumber, sizeof(SPersistentItem), &bytes_written, NULL);

  CloseHandle(file);
}

////////////////////////////////////////////////////////////////////////////////

byte*
_PERSISTENT::GetData(int itemnumber)
{
  return Items[itemnumber].data;
}

////////////////////////////////////////////////////////////////////////////////

