//#include <windows.h>
#include "SwatchServer.hpp"
#include <wx/config.h>
#include "Configuration.hpp"


CSwatchServer* CSwatchServer::s_Server;


////////////////////////////////////////////////////////////////////////////////

CSwatchServer*
CSwatchServer::Instance()
{
  if (!s_Server) {
    s_Server = new CSwatchServer();
    atexit(OnExitHandler);
  }
  return s_Server;
}

////////////////////////////////////////////////////////////////////////////////

CSwatchServer::CSwatchServer()
{
  wxConfig *config = new wxConfig("Sphere");
  wxString value;
  config->SetPath("/SwatchColors");
  value = config->Read("Colors", "");
  int len = value.Len();
  len -= len % 8;
  int i;
  RGBA color;
  for(i = 0; i < len; i += 8) {
    color.red =   Configuration::FromHex(value.GetChar(i + 0)) << 4 | Configuration::FromHex(value.GetChar(i + 1));
    color.green = Configuration::FromHex(value.GetChar(i + 2)) << 4 | Configuration::FromHex(value.GetChar(i + 3));
    color.blue =  Configuration::FromHex(value.GetChar(i + 4)) << 4 | Configuration::FromHex(value.GetChar(i + 5));
    color.alpha = Configuration::FromHex(value.GetChar(i + 6)) << 4 | Configuration::FromHex(value.GetChar(i + 7));
    m_Colors.push_back(color);
  }
  delete config;
/*
  // load the swatch
  HKEY key;
  if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\AegisKnight\\Sphere", 0, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS) {
    return;
  }

  // find out how big the swatch buffer is
  DWORD size = 0;
  RegQueryValueEx(key, "SwatchColors", NULL, NULL, NULL, &size);

  if (size == 0) {
    RegCloseKey(key);
    return;
  }

  // read the swatch buffer
  RGBA* buffer = new RGBA[(size + 3) / 4];
  RegQueryValueEx(key, "SwatchColors", NULL, NULL, (LPBYTE)buffer, &size);

  // parse it and add to the server
  RGBA* p = buffer;
  for (int i = 0; i < size / 4; i++) {
    m_Colors.push_back(*p++);
  }
  
  delete[] buffer;
  RegCloseKey(key);
*/
}

////////////////////////////////////////////////////////////////////////////////

CSwatchServer::~CSwatchServer()
{
  wxConfig *config = new wxConfig("Sphere");
  wxString value;
  config->SetPath("/SwatchColors");
  int len = m_Colors.size();
  int i;
  for(i = 0; i < len; i++) {
    value.Append(Configuration::ToHex((m_Colors[i].red >> 4) & 0xf));
    value.Append(Configuration::ToHex(m_Colors[i].red & 0xf));
    value.Append(Configuration::ToHex((m_Colors[i].green >> 4) & 0xf));
    value.Append(Configuration::ToHex(m_Colors[i].green & 0xf));
    value.Append(Configuration::ToHex((m_Colors[i].blue >> 4) & 0xf));
    value.Append(Configuration::ToHex(m_Colors[i].blue & 0xf));
    value.Append(Configuration::ToHex((m_Colors[i].alpha >> 4) & 0xf));
    value.Append(Configuration::ToHex(m_Colors[i].alpha & 0xf));

  }
  config->Write("Colors", value);
  delete config;
/*
  // save the swatch
  HKEY key;
  if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\AegisKnight\\Sphere", 0, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS) {
    return;
  }

  // make the swatch buffer
  RGBA* buffer = new RGBA[m_Colors.size()];
  for (int i = 0; i < m_Colors.size(); i++) {
    buffer[i] = m_Colors[i];
  }

  // write the swatch buffer
  RegSetValueEx(key, "SwatchColors", 0, REG_BINARY, (BYTE*)buffer, m_Colors.size() * sizeof(RGBA));

  delete[] buffer;
  RegCloseKey(key);
*/
}

////////////////////////////////////////////////////////////////////////////////

void __cdecl
CSwatchServer::OnExitHandler()
{
  delete s_Server;
}

////////////////////////////////////////////////////////////////////////////////

int
CSwatchServer::GetNumColors() const
{
  return m_Colors.size();
}

////////////////////////////////////////////////////////////////////////////////

RGBA
CSwatchServer::GetColor(int i) const
{
  return m_Colors[i];
}

////////////////////////////////////////////////////////////////////////////////

void
CSwatchServer::Clear()
{
  m_Colors.clear();
}

////////////////////////////////////////////////////////////////////////////////

void
CSwatchServer::SetColor(int i, RGBA color)
{
  if (i >= 0 && i < m_Colors.size()) {
    m_Colors[i] = color;
  } else {
    m_Colors.push_back(color);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CSwatchServer::InsertColor(int i, RGBA color)
{
  if (i > m_Colors.size()) {
    i = m_Colors.size();
  }
  m_Colors.insert(m_Colors.begin() + i, color);
}

////////////////////////////////////////////////////////////////////////////////

void
CSwatchServer::DeleteColor(int i)
{
  if (i >= 0 && i < m_Colors.size()) {
    m_Colors.erase(m_Colors.begin() + i);
  }
}

////////////////////////////////////////////////////////////////////////////////
