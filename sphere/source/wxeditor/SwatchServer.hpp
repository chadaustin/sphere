#ifndef SWATCH_SERVER_HPP
#define SWATCH_SERVER_HPP


#include <vector>
#include "../common/rgb.hpp"


// singleton

class CSwatchServer
{
public:
  static CSwatchServer* Instance();

  int GetNumColors() const;
  RGBA GetColor(int i) const;

  void Clear();
  void SetColor(int i, RGBA color);
  void InsertColor(int i, RGBA color);
  void DeleteColor(int i);

private:
  CSwatchServer();
  ~CSwatchServer();

#ifdef WIN32
  static void __cdecl OnExitHandler();
#endif

  std::vector<RGBA> m_Colors;

  static CSwatchServer* s_Server;
};


#endif
