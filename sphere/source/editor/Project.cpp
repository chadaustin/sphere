#pragma warning(disable : 4786)  // identifier too long


#include <windows.h>
#include <set>
#include <stdio.h>
#include <string.h>
#include "Project.hpp"
#include "../common/configfile.hpp"
#include "../common/types.h"


////////////////////////////////////////////////////////////////////////////////

CProject::CProject()
: m_ScreenWidth(320)
, m_ScreenHeight(240)
{
  m_GameTitle   = "Untitled";
  m_Author      = "Unknown";
}

////////////////////////////////////////////////////////////////////////////////

bool
CProject::Create(const char* games_directory, const char* project_name)
{
  // create the project
  if (SetCurrentDirectory(games_directory) == FALSE)
    return false;
  
  // if creating the directory failed, it may already exist
  CreateDirectory(project_name, NULL);

  // now create all of the subdirectories
  for (int i = 0; i < NUM_GROUP_TYPES; i++)
  {
    std::string directory = project_name;
    directory += "/";
    directory += GetGroupDirectory(i);
    CreateDirectory(directory.c_str(), NULL);
  }
  
  // wait to see if SetCurrentDirectory() fails
  if (SetCurrentDirectory(project_name) == FALSE)
    return false;

  // set the project directory
  char path[MAX_PATH];
  if (GetCurrentDirectory(MAX_PATH, path) == FALSE)
    return false;

  m_Directory = path;
    
  // set the project filename
  m_Filename = path;
  m_Filename += "\\game.sgm";

  // set default values in project
  m_GameTitle   = "Untitled";
  m_Author      = "Unknown";
  m_Description = "";

  m_GameScript = "";
  m_ScreenWidth = 320;
  m_ScreenHeight = 240;

  RefreshItems();
  return Save();
}

////////////////////////////////////////////////////////////////////////////////

bool
CProject::Open(const char* filename)
{
  Destroy();

  // set the game directory
  m_Directory = filename;
  if (m_Directory.rfind('\\') != std::string::npos)
    m_Directory[m_Directory.rfind('\\')] = 0;

  // set the game filename
  m_Filename = filename;

  // load the game.sgm
  CConfigFile config(m_Filename.c_str());

  m_GameTitle   = config.ReadString("", "name",        "Untitled");
  m_Author      = config.ReadString("", "author",      "Unknown");
  m_Description = config.ReadString("", "description", "");

  m_GameScript = config.ReadString("", "script", "");

  // screen dimensions
  m_ScreenWidth  = config.ReadInt("", "screen_width", 320);
  m_ScreenHeight = config.ReadInt("", "screen_height", 240);

  RefreshItems();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CProject::Save() const
{
  SetCurrentDirectory(m_Directory.c_str());
  
  CConfigFile config;

  config.WriteString("", "name", m_GameTitle.c_str());
  config.WriteString("", "author", m_Author.c_str());
  config.WriteString("", "description", m_Description.c_str());

  config.WriteString("", "script", m_GameScript.c_str());

  // screen dimensions
  config.WriteInt("", "screen_width",  m_ScreenWidth);
  config.WriteInt("", "screen_height", m_ScreenHeight);

  config.Save(m_Filename.c_str());
  return true;
}

////////////////////////////////////////////////////////////////////////////////

const char*
CProject::GetDirectory() const
{
  return m_Directory.c_str();
}

////////////////////////////////////////////////////////////////////////////////

const char*
CProject::GetGameSubDirectory() const
{
  if (strrchr(m_Directory.c_str(), '\\'))
    return strrchr(m_Directory.c_str(), '\\') + 1;
  else
    return "";
}

////////////////////////////////////////////////////////////////////////////////

const char*
CProject::GetGameTitle() const
{
  return m_GameTitle.c_str();
}

////////////////////////////////////////////////////////////////////////////////

const char*
CProject::GetAuthor() const
{
  return m_Author.c_str();
}

////////////////////////////////////////////////////////////////////////////////

const char*
CProject::GetDescription() const
{
  return m_Description.c_str();
}

////////////////////////////////////////////////////////////////////////////////

const char*
CProject::GetGameScript() const
{
  return m_GameScript.c_str();
}

////////////////////////////////////////////////////////////////////////////////

int
CProject::GetScreenWidth() const
{
  return m_ScreenWidth;
}

////////////////////////////////////////////////////////////////////////////////

int
CProject::GetScreenHeight() const
{
  return m_ScreenHeight;
}

////////////////////////////////////////////////////////////////////////////////

void
CProject::SetGameTitle(const char* game_title)
{
  m_GameTitle = game_title;
}

////////////////////////////////////////////////////////////////////////////////

void
CProject::SetAuthor(const char* author)
{
  m_Author = author;
}

////////////////////////////////////////////////////////////////////////////////

void
CProject::SetDescription(const char* description)
{
  m_Description = description;
}

////////////////////////////////////////////////////////////////////////////////

void
CProject::SetGameScript(const char* game_script)
{
  m_GameScript = game_script;
}

////////////////////////////////////////////////////////////////////////////////

void
CProject::SetScreenWidth(int width)
{
  m_ScreenWidth = width;
}

////////////////////////////////////////////////////////////////////////////////

void
CProject::SetScreenHeight(int height)
{
  m_ScreenHeight = height;
}

////////////////////////////////////////////////////////////////////////////////

const char*
CProject::GetGroupDirectory(int grouptype)
{
  switch (grouptype)
  {
    case GT_MAPS:         return "maps";
    case GT_SPRITESETS:   return "spritesets";
    case GT_SCRIPTS:      return "scripts";
    case GT_SOUNDS:       return "sounds";
    case GT_FONTS:        return "fonts";
    case GT_WINDOWSTYLES: return "windowstyles";
    case GT_IMAGES:       return "images";
    case GT_ANIMATIONS:   return "animations";
    default:              return NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CProject::RefreshItems()
{
  // empty the old lists
  for (int i = 0; i < NUM_GROUP_TYPES; i++)
    m_Groups[i].clear();

  // store the old directory
  char old_directory[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, old_directory);

  for (int i = 0; i < NUM_GROUP_TYPES; i++)
  {
    SetCurrentDirectory(m_Directory.c_str());
    
    if (!SetCurrentDirectory(GetGroupDirectory(i)))
      continue;

    // add all extensions to this set
    std::vector<std::string> extensions;
    FTL.GetFileTypeExtensions(i, extensions);

    for (int j = 0; j < extensions.size(); j++) {
      std::string filter = "*." + extensions[j];

      WIN32_FIND_DATA ffd;
      HANDLE h = FindFirstFile(filter.c_str(), &ffd);
      if (h == INVALID_HANDLE_VALUE)
        continue;

      do {

        if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
          AddItem(i, ffd.cFileName);

      } while (FindNextFile(h, &ffd));

      FindClose(h);
    }

  }

  // restore the old directory
  SetCurrentDirectory(old_directory);
}

////////////////////////////////////////////////////////////////////////////////

int
CProject::GetItemCount(int group_type) const
{
  return m_Groups[group_type].size();
}

////////////////////////////////////////////////////////////////////////////////

const char*
CProject::GetItem(int group_type, int i) const
{
  return m_Groups[group_type][i].c_str();
}

////////////////////////////////////////////////////////////////////////////////

bool
CProject::HasItem(int group_type, const char* item) const
{
  for (int i = 0; i < GetItemCount(group_type); i++)
    if (strcmp(item, GetItem(group_type, i)) == 0)
      return true;
  return false;
}

////////////////////////////////////////////////////////////////////////////////

void
CProject::AddItem(int grouptype, const char* filename)
{
  Group& group = m_Groups[grouptype];

  // make sure it's not in the group already
  for (int i = 0; i < group.size(); i++)
    if (filename == group[i])
      return;

  // if we're adding a script to the project and we don't have a game script,
  // set it.
  if (grouptype == GT_SCRIPTS && m_GameScript.empty()) {
    m_GameScript = filename;
    if (!m_Filename.empty()) {
      Save();
    }
  }

  group.push_back(filename);
}

////////////////////////////////////////////////////////////////////////////////

void
CProject::Destroy()
{
  m_Directory  = "";
  m_Filename   = "";
  m_GameTitle  = "";
  m_GameScript = "";

  m_ScreenWidth = 0;
  m_ScreenHeight = 0;

  for (int i = 0; i < NUM_GROUP_TYPES; i++)
    m_Groups[i].clear();
}

////////////////////////////////////////////////////////////////////////////////
