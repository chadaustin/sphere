#ifndef SMAP_HPP
#define SMAP_HPP


#include <vector>
#include "video.hpp"
#include "../common/Map.hpp"
#include "../common/Layer.hpp"
#include "../common/Tileset.hpp"
#include "../common/IFileSystem.hpp"


class SMAP
{
public:
  SMAP();
  ~SMAP();

  bool Load(const char* filename, IFileSystem& fs);
  void UpdateMap();

  sMap& GetMap();

  void RenderLayer(int i, bool solid, int camera_x, int camera_y, int& offset_x, int& offset_y);

  void SetLayerAlpha(int layer, int alpha);
  int  GetLayerAlpha(int layer);

private:
  struct TileDelay {
    int current;
    int delay;
    int next;
  };

private:
  void InitializeAnimation();

private:
  sMap m_Map;
  std::vector<TileDelay> m_AnimationMap;

  std::vector<IMAGE> m_Tiles;
  std::vector<IMAGE> m_SolidTiles; // used for reflectivity
  std::vector<int> m_LayerTimes;
  std::vector<int> m_LayerAlphas;

  // max dimensions of non-parallax layers
  int m_MaxLayerWidth;
  int m_MaxLayerHeight;
};


////////////////////////////////////////////////////////////////////////////////

inline sMap&
SMAP::GetMap()
{
  return m_Map;
}

////////////////////////////////////////////////////////////////////////////////


#endif
