#ifndef __CONFIG_H
#define __CONFIG_H


#include "types.h"


typedef struct
{
  char* key;
  char* value;
} _ITEM;


typedef struct
{
  char*  name;
  int    numitems;
  _ITEM* items;
} _SECTION;


typedef struct
{
  int       numsections;
  _SECTION* sections;
} CONFIG;


#include "begin_c_prototypes.h"

  // core functions
  bool CreateConfig(CONFIG* config);
  bool DestroyConfig(CONFIG* config);

  bool LoadConfig(CONFIG* config, const char* filename);
  bool SaveConfig(const CONFIG* config, const char* filename);

  bool ReadConfigString(const CONFIG* config, const char* section, const char* key, char* string, int length, const char* def);
  bool ReadConfigInt(const CONFIG* config, const char* section, const char* key, int* i, int def);

  bool WriteConfigString(CONFIG* config, const char* section, const char* key, const char* string);
  bool WriteConfigInt(CONFIG* config, const char* section, const char* key, int i);

  // quick functions
  bool ReadConfigFileString(const char* filename, const char* section, const char* key, char* string, int length, const char* def);
  bool ReadConfigFileInt(   const char* filename, const char* section, const char* key, int* i, int def);
  bool ReadConfigFileBool(  const char* filename, const char* section, const char* key, bool* b, bool def);

  bool WriteConfigFileString(const char* filename, const char* section, const char* key, const char* string);
  bool WriteConfigFileInt(   const char* filename, const char* section, const char* key, int i);
  bool WriteConfigFileBool(  const char* filename, const char* section, const char* key, bool b);

#include "end_c_prototypes.h"


#endif
