#include "spritesetserver.hpp"


////////////////////////////////////////////////////////////////////////////////

CSpritesetServer::CSpritesetServer()
{
}

////////////////////////////////////////////////////////////////////////////////

CSpritesetServer::~CSpritesetServer()
{
  for (int i = 0; i < m_Spritesets.size(); i++)
    delete m_Spritesets[i].spriteset;
}

////////////////////////////////////////////////////////////////////////////////

SSPRITESET*
CSpritesetServer::Load(const char* filename, IFileSystem& fs)
{
  // if a script is already loaded...
  for (int i = 0; i < m_Spritesets.size(); i++)
    if (filename == m_Spritesets[i].name)
    {
      m_Spritesets[i].refcount++;
      return m_Spritesets[i].spriteset;
    }
  
  // we have to load a new one...
  Spriteset spriteset;
  spriteset.name      = filename;
  spriteset.refcount  = 1;
  spriteset.spriteset = new SSPRITESET;
  m_Spritesets.push_back(spriteset);

  if (!spriteset.spriteset->Load(filename, fs))
  {
    delete spriteset.spriteset;
    m_Spritesets.pop_back();
    return NULL;
  }

  return spriteset.spriteset;
}

////////////////////////////////////////////////////////////////////////////////

void
CSpritesetServer::Free(SSPRITESET* spriteset)
{
  // find the script
  for (int i = 0; i < m_Spritesets.size(); i++)
    if (spriteset == m_Spritesets[i].spriteset)
      if (--m_Spritesets[i].refcount == 0)
      {
        // remove m_Spritesets[i]
        delete m_Spritesets[i].spriteset;
        m_Spritesets.erase(m_Spritesets.begin() + i);
        return;
      }
}

////////////////////////////////////////////////////////////////////////////////
