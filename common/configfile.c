#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "configfile.h"


////////////////////////////////////////////////////////////////////////////////

static void skip_whitespace(char* string)
{
  int first_non_whitespace = 0;
  while (isspace(string[first_non_whitespace]))
    first_non_whitespace++;
  memmove(string, string + first_non_whitespace, strlen(string + first_non_whitespace));
}

////////////////////////////////////////////////////////////////////////////////

static _SECTION* FindSection(const CONFIG* config, const char* section)
{
  int i;
  for (i = 0; i < config->numsections; i++)
    if (strcmp(section, config->sections[i].name) == 0)
      return config->sections + i;
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

static _ITEM* FindItem(const _SECTION* section, const char* key)
{
  int i;
  for (i = 0; i < section->numitems; i++)
    if (strcmp(key, section->items[i].key) == 0)
      return section->items + i;
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

static _SECTION* GetSection(CONFIG* config, const char* section)
{
  _SECTION* s = FindSection(config, section);
  if (s == NULL)
  {
    config->sections = (_SECTION*)realloc(config->sections, (config->numsections + 1) * sizeof(_SECTION));
    config->sections[config->numsections].name = strdup(section);
    config->sections[config->numsections].numitems = 0;
    config->sections[config->numsections].items = NULL;
    config->numsections++;
    return config->sections + config->numsections - 1;
  }
  else
    return s;
}

////////////////////////////////////////////////////////////////////////////////

static _ITEM* GetItem(_SECTION* section, const char* key)
{
  _ITEM* i = FindItem(section, key);
  if (i == NULL)
  {
    section->items = (_ITEM*)realloc(section->items, (section->numitems + 1) * sizeof(_ITEM));
    section->items[section->numitems].key = strdup(key);;
    section->items[section->numitems].value = strdup("");
    section->numitems++;
    return section->items + section->numitems - 1;
  }
  else
    return i;
}

////////////////////////////////////////////////////////////////////////////////

bool CreateConfig(CONFIG* config)
{
  config->numsections = 0;
  config->sections = NULL;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DestroyConfig(CONFIG* config)
{
  int i;
  for (i = 0; i < config->numsections; i++)
  {
    int j;
    for (j = 0; j < config->sections[i].numitems; j++)
    {
      free(config->sections[i].items[j].key);
      free(config->sections[i].items[j].value);
    }
    if (config->sections[i].numitems)
      free(config->sections[i].items);
    free(config->sections[i].name);
  }
  if (config->numsections)
    free(config->sections);
  config->numsections = 0;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool LoadConfig(CONFIG* config, const char* filename)
{
  FILE* file = fopen(filename, "r");

  // blank it out
  CreateConfig(config);

  // if file doesn't exist, config is blank
  if (file == NULL)
  {
    config->numsections = 0;
    config->sections    = NULL;
  }
  else
  {
    char current_section[513];
    bool eof;

    // default section is ""
    strcpy(current_section, "");

    eof = false;
    while (!eof)
    {
      char string[513];

      // read a line
      if (fgets(string, 512, file) == NULL)
        eof = true;
      else
      {
        // parse it
        
        // replace end-of-line with end-of-string
        if (strchr(string, '\n'))
          *strchr(string, '\n') = 0;

        // eliminate whitespace
        skip_whitespace(string);

        if (string[0] == '[')  // it's a section
        {
          // get the name of the section
          if (strchr(string + 1, ']'))
          {
            *strchr(string + 1, ']') = 0;
            strcpy(current_section, string + 1);
          }
        }
        else                  // it's a key=value pair
        {
          char key[513];
          char value[513];
          char* equals = strchr(string, '=');
          if (equals)
          {
            *equals = 0;
            strcpy(key,   string);
            strcpy(value, string + strlen(string) + 1);
          }

          // add the item
          WriteConfigString(config, current_section, key, value);
        }
      }

    }

    fclose(file);
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool SaveConfig(const CONFIG* config, const char* filename)
{
  int section;

  FILE* file = fopen(filename, "w");
  if (file == NULL)
    return false;

  // find the section without a name and write that first
  for (section = 0; section < config->numsections; section++)
    if (strcmp(config->sections[section].name, "") == 0)
    {
      int item;
      for (item = 0; item < config->sections[section].numitems; item++)
      {
        fputs(config->sections[section].items[item].key, file);
        fputc('=', file);
        fputs(config->sections[section].items[item].value, file);
        fputc('\n', file);
      }

      fputc('\n', file);
    }
  
  for (section = 0; section < config->numsections; section++)
    if (strcmp(config->sections[section].name, "") != 0)
    {
      int item;
      
      // write the section name
      fputc('[', file);
      fputs(config->sections[section].name, file);
      fputs("]\n", file);
  
      // write the values
      for (item = 0; item < config->sections[section].numitems; item++)
      {
        fputs(config->sections[section].items[item].key, file);
        fputc('=', file);
        fputs(config->sections[section].items[item].value, file);
        fputc('\n', file);
      }

      fputc('\n', file);
    }

  fclose(file);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool ReadConfigString(const CONFIG* config, const char* section, const char* key, char* string, int length, const char* def)
{
  _SECTION* s = FindSection(config, section);
  if (s != NULL)
  {
    _ITEM* i = FindItem(s, key);
    if (i != NULL)
    {
      strncpy(string, i->value, length);
      return true;
    }
  }

  strncpy(string, def, length);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool ReadConfigInt(const CONFIG* config, const char* section, const char* key, int* i, int def)
{
  _SECTION* s = FindSection(config, section);
  if (s != NULL)
  {
    _ITEM* item = FindItem(s, key);
    if (item != NULL)
    {
      *i = atoi(item->value);
      return true;
    }
  }

  *i = def;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool WriteConfigString(CONFIG* config, const char* section, const char* key, const char* string)
{
  _SECTION* s = GetSection(config, section);
  _ITEM*    i = GetItem(s, key);
  
  free(i->value);
  i->value = strdup(string);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool WriteConfigInt(CONFIG* config, const char* section, const char* key, int i)
{
  char string[80];
  sprintf(string, "%d", i);
  WriteConfigString(config, section, key, string);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool ReadConfigFileString(const char* filename, const char* section, const char* key, char* string, int length, const char* def)
{
  CONFIG config;
  strncpy(string, def, length);

  if (!LoadConfig(&config, filename))
    return false;

  ReadConfigString(&config, section, key, string, length, def);

  DestroyConfig(&config);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool ReadConfigFileInt(const char* filename, const char* section, const char* key, int* i, int def)
{
  CONFIG config;
  *i = def;

  if (!LoadConfig(&config, filename))
    return false;

  ReadConfigInt(&config, section, key, i, def);

  DestroyConfig(&config);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool ReadConfigFileBool(const char* filename, const char* section, const char* key, bool* b, bool def)
{
  int i;
  ReadConfigFileInt(filename, section, key, &i, def);
  *b = (i != 0);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool WriteConfigFileString(const char* filename, const char* section, const char* key, const char* string)
{
  CONFIG config;
  if (!LoadConfig(&config, filename))
    return false;

  WriteConfigString(&config, section, key, string);

  SaveConfig(&config, filename);
  DestroyConfig(&config);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool WriteConfigFileInt(const char* filename, const char* section, const char* key, int i)
{
  CONFIG config;
  if (!LoadConfig(&config, filename))
    return false;

  WriteConfigInt(&config, section, key, i);

  SaveConfig(&config, filename);
  DestroyConfig(&config);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool WriteConfigFileBool(const char* filename, const char* section, const char* key, bool b)
{
  return WriteConfigFileInt(filename, section, key, b);
}

////////////////////////////////////////////////////////////////////////////////
