#ifndef RENDER_SORT_HPP
#define RENDER_SORT_HPP


#include <vector>
#include "video.hpp"


class CRenderSort
{
public:
  void AddObject(int draw_x, int draw_y, int sort_y, int draw_w, int draw_h, IMAGE image);
  void DrawObjects();

private:
  struct Object
  {
    int   draw_x;
    int   draw_y;
    int   sort_y;
    int   draw_w;
    int   draw_h;
    IMAGE image;
  };

private:
  std::vector<Object> m_objects;
};


#endif
