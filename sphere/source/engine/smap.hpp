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
  struct TileDelay {
    int current; ///< the current tile that is displayed
    int delay;   ///< delay until next switch
    int next;    ///< next tile to be displayed
  };

  SMAP();
  ~SMAP();

  bool Load(const char* filename, IFileSystem& fs);
  void UpdateMap();

  sMap& GetMap();

  void RenderLayer(int i, bool solid, int camera_x, int camera_y, int& offset_x, int& offset_y);
  int MapToScreenX(int layer, int camera_x, int mx);
  int MapToScreenY(int layer, int camera_y, int my);
  int ScreenToMapX(int layer, int camera_x, int sx);
  int ScreenToMapY(int layer, int camera_y, int sy);

  void SetLayerAlpha(int layer, int alpha);
  int  GetLayerAlpha(int layer);

  const std::vector<TileDelay>& GetAnimationMap() const {
    return m_AnimationMap;
  }

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
