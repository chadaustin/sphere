#include <string.h>
#include "Tile.hpp"
#include "Tileset.hpp"

////////////////////////////////////////////////////////////////////////////////

sTile::sTile(int width, int height)
: CImage32(width, height)
, m_Animated(false)
, m_NextTile(0)
, m_Delay(0)
, m_CurrentShown(0)
, m_UpdateFrame(0)
{
  Clear();
}

////////////////////////////////////////////////////////////////////////////////

sTile::sTile(const sTile& rhs)
{
  CImage32::operator=(rhs);

  m_Animated       = rhs.m_Animated;
  m_NextTile       = rhs.m_NextTile;
  m_Delay          = rhs.m_Delay;
  m_ObstructionMap = rhs.m_ObstructionMap;
  m_Name           = rhs.m_Name;
	m_CurrentShown   = rhs.m_CurrentShown;
	m_UpdateFrame    = rhs.m_UpdateFrame;	
}

////////////////////////////////////////////////////////////////////////////////

sTile&
sTile::operator=(const sTile& rhs)
{
  CImage32::operator=(rhs);

  m_Animated       = rhs.m_Animated;
  m_NextTile       = rhs.m_NextTile;
  m_Delay          = rhs.m_Delay;
  m_ObstructionMap = rhs.m_ObstructionMap;
  m_Name           = rhs.m_Name;
	m_CurrentShown   = rhs.m_CurrentShown;
	m_UpdateFrame    = rhs.m_UpdateFrame;	

  return *this;
}

////////////////////////////////////////////////////////////////////////////////

void
sTile::SetAnimated(bool animated)
{
  m_Animated = animated;
}

////////////////////////////////////////////////////////////////////////////////

void
sTile::SetNextTile(int next_tile)
{
  m_NextTile = next_tile;
}

////////////////////////////////////////////////////////////////////////////////

void
sTile::SetDelay(int delay)
{
  m_Delay = delay;
}

////////////////////////////////////////////////////////////////////////////////

void
sTile::SetName(std::string tile_name)
{
  m_Name = tile_name;
}

////////////////////////////////////////////////////////////////////////////////

void
sTile::UpdateAnimation(int frame, const sTileset & parent)
{
	m_Updated = false;

	const sTile * pShown = &parent.GetTile(m_CurrentShown);
	while (pShown->IsAnimated() && frame > m_UpdateFrame)
	{
		m_Updated = true;
		m_CurrentShown = pShown->GetNextTile();		
		pShown = &parent.GetTile(m_CurrentShown);
		m_UpdateFrame += pShown->GetDelay();
	}	
}

void 
sTile::InitAnimation(int thisTile, int frame)
{
	m_CurrentShown = thisTile;
	m_UpdateFrame  = frame + m_Delay;
}