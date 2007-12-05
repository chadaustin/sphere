#ifdef _MSC_VER
#pragma warning(disable : 4786)  // identifier too long
#endif

#include <wx/wx.h>

#include <set>
#include <stdio.h>
#include <string.h>
#include "Project.hpp"
#include "../common/configfile.hpp"
#include "../common/types.h"

#include "../common/system.hpp"

////////////////////////////////////////////////////////////////////////////////

wProject::wProject()
: m_ScreenWidth(320)
, m_ScreenHeight(240)
{
}

////////////////////////////////////////////////////////////////////////////////

bool
wProject::Create(const char* games_directory, const char* project_name)
{
  // create the project
  if (wxSetWorkingDirectory(games_directory) == FALSE) {
    ::wxMessageBox("Could not chdir to game directory");
    return false;
  }

  // if creating the directory failed, it may already exist
  if (!wxPathExists(project_name)) wxMkdir(project_name, 0755);

  // now create all of the subdirectories
  for (int i = 0; i < NUM_GROUP_TYPES; i++)
  {
    std::string directory = project_name;
    directory += "/";
    directory += GetGroupDirectory(i);
    if (!wxPathExists(directory.c_str())) wxMkdir(directory.c_str(), 0755);
  }
  // create an 'other' subdirectory too
  std::string directory = project_name;
  directory += "/other";
  if (!wxPathExists(directory.c_str())) wxMkdir(directory.c_str(), 0755);
  
  // wait to see if wxSetWorkingDirectory() fails
  if (!wxSetWorkingDirectory(project_name))
    return false;

  // set the project directory
  char path[MAX_PATH];
  if (GetCurrentDirectory(MAX_PATH, path) == FALSE)
    return false;
  m_Directory = path;
    
  // set the project filename
  m_Filename = path;
  m_Filename += "/game.sgm";

  // for reloading -if- they don't change game.sgm before quiting.
  std::ofstream output(m_Filename.c_str());
  output << "-";
  output.close();

  // set default values in project
  m_GameTitle = "";
  m_GameScript = "";

  m_ScreenWidth = 320;
  m_ScreenHeight = 240;

  RefreshItems();
  return Save();
}

////////////////////////////////////////////////////////////////////////////////

bool
wProject::Open(const char* filename)
{
  Destroy();

  // set the game directory
  m_Directory = filename;

#ifdef WIN32
  if (m_Directory.rfind('\\') != std::string::npos)
    m_Directory[m_Directory.rfind('\\')] = 0;
#else
  if (m_Directory.rfind('/') != std::string::npos)
    m_Directory[m_Directory.rfind('/')] = 0;
#endif

  // set the game filename
  m_Filename = filename;

  // load the game.sgm
  CConfigFile config(m_Filename.c_str());

  m_GameTitle = config.ReadString("", "name", "");
  m_GameScript = config.ReadString("", "script", "");

  // screen dimensions
  m_ScreenWidth = config.ReadInt("", "screen_width", 320);
  m_ScreenHeight = config.ReadInt("", "screen_height", 240);

  RefreshItems();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
wProject::Save() const
{
  wxSetWorkingDirectory(m_Directory.c_str());
  
  CConfigFile config;

  config.WriteString("", "name", m_GameTitle.c_str());
  config.WriteString("", "script", m_GameScript.c_str());

  // screen dimensions
  config.WriteInt("", "screen_width",  m_ScreenWidth);
  config.WriteInt("", "screen_height", m_ScreenHeight);

  config.Save(m_Filename.c_str());
  return true;
}

////////////////////////////////////////////////////////////////////////////////

const char*
wProject::GetDirectory() const
{
  return m_Directory.c_str();
}

////////////////////////////////////////////////////////////////////////////////

const char*
wProject::GetGameSubDirectory() const
{
#ifdef WIN32
  if (strrchr(m_Directory.c_str(), '\\'))
    return strrchr(m_Directory.c_str(), '\\') + 1;
#else
  if (strrchr(m_Directory.c_str(), '/'))
    return strrchr(m_Directory.c_str(), '/') + 1;
#endif
  else
    return "";
}

////////////////////////////////////////////////////////////////////////////////

const char*
wProject::GetGameTitle() const
{
  return m_GameTitle.c_str();
}

////////////////////////////////////////////////////////////////////////////////

const char*
wProject::GetGameScript() const
{
  return m_GameScript.c_str();
}

////////////////////////////////////////////////////////////////////////////////

int
wProject::GetScreenWidth() const
{
  return m_ScreenWidth;
}

////////////////////////////////////////////////////////////////////////////////

int
wProject::GetScreenHeight() const
{
  return m_ScreenHeight;
}

////////////////////////////////////////////////////////////////////////////////

void
wProject::SetGameTitle(const char* game_title)
{
  m_GameTitle = game_title;
}

////////////////////////////////////////////////////////////////////////////////

void
wProject::SetGameScript(const char* game_script)
{
  m_GameScript = game_script;
}

////////////////////////////////////////////////////////////////////////////////

void
wProject::SetScreenWidth(int width)
{
  m_ScreenWidth = width;
}

////////////////////////////////////////////////////////////////////////////////

void
wProject::SetScreenHeight(int height)
{
  m_ScreenHeight = height;
}

////////////////////////////////////////////////////////////////////////////////

const char*
wProject::GetGroupDirectory(int grouptype)
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
wProject::RefreshItems()
{
  // empty the old lists
  for (int i = 0; i < NUM_GROUP_TYPES; i++)
    m_Groups[i].clear();

  // store the old directory
  char old_directory[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, old_directory);

  for (int i = 0; i < NUM_GROUP_TYPES; i++)
  {
    wxSetWorkingDirectory(m_Directory.c_str());
    
    if (!wxSetWorkingDirectory(GetGroupDirectory(i)))
      continue;

    // add all extensions to this set
    std::vector<std::string> extensions;
    FTL.GetFileTypeExtensions(i, extensions);

    for (unsigned int j = 0; j < extensions.size(); j++) {
      std::string filter = "*." + extensions[j];

      std::vector<std::string> file_list = GetFileList(filter.c_str());
      for (unsigned int m = 0; m < file_list.size(); m++) {
        AddItem(i, file_list[m].c_str());
      }
    }

  }

  // restore the old directory
  wxSetWorkingDirectory(old_directory);
}

////////////////////////////////////////////////////////////////////////////////

int
wProject::GetItemCount(int group_type) const
{
  return m_Groups[group_type].size();
}

////////////////////////////////////////////////////////////////////////////////

const char*
wProject::GetItem(int group_type, int i) const
{
  return m_Groups[group_type][i].c_str();
}

////////////////////////////////////////////////////////////////////////////////

bool
wProject::HasItem(int group_type, const char* item) const
{
  for (int i = 0; i < GetItemCount(group_type); i++)
    if (strcmp(item, GetItem(group_type, i)) == 0)
      return true;
  return false;
}

////////////////////////////////////////////////////////////////////////////////

void
wProject::AddItem(int grouptype, const char* filename)
{
  Group& group = m_Groups[grouptype];

  // make sure it's not in the group already
  for (unsigned int i = 0; i < group.size(); i++)
    if (filename == group[i])
      return;

  group.push_back(filename);
}

////////////////////////////////////////////////////////////////////////////////

void
wProject::Destroy()
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
