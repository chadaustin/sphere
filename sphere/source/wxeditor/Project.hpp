#ifndef PROJECT_HPP
#define PROJECT_HPP


#ifdef _MSC_VER
// disable 'identifier too long'
#pragma warning(disable : 4786)
#endif


#include <vector>
#include <string>
#include "FileTypes.hpp"


class wProject
{
public:
  wProject();

  bool Create(const char* games_directory, const char* project_name);
  bool Open(const char* filename);
  bool Save() const;

  const char* GetDirectory() const;        // returns full path to game
  const char* GetGameSubDirectory() const;

  const char* GetGameTitle() const;
  const char* GetGameScript() const;
  int         GetScreenWidth() const;
  int         GetScreenHeight() const;

  void SetGameTitle(const char* game_title);
  void SetGameScript(const char* game_script);
  void SetScreenWidth(int width);
  void SetScreenHeight(int height);

  static const char* GetGroupDirectory(int grouptype); // returns relative path

  void RefreshItems();

  int         GetItemCount(int grouptype) const;
  const char* GetItem(int grouptype, int index) const;
  bool        HasItem(int grouptype, const char* item) const;

private:
  typedef std::vector<std::string> Group;

private:
  void AddItem(int grouptype, const char* filename);
  void Destroy();

private:
  std::string m_Directory;
  std::string m_Filename;

  std::string m_GameTitle;
  std::string m_GameScript;

  int m_ScreenWidth;
  int m_ScreenHeight;

  Group m_Groups[NUM_GROUP_TYPES];
};


#endif
