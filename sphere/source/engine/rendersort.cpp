#include <algorithm>
#include <math.h>
#include "rendersort.hpp"




////////////////////////////////////////////////////////////////////////////////

void
CRenderSort::AddObject(int draw_x, int draw_y, int sort_y, int draw_w, int draw_h, bool is_angled, double angle, IMAGE image, RGBA mask)
{
  Object object;
  object.draw_x = draw_x;
  object.draw_y = draw_y;
  object.sort_y = sort_y;
  object.draw_w = draw_w;
  object.draw_h = draw_h;
  object.is_angled = is_angled;
  object.angle = angle;
  object.image  = image;
  object.mask   = mask;
  m_objects.push_back(object);
}

////////////////////////////////////////////////////////////////////////////////

void CalculateRotateBlitPoints(int tx[4], int ty[4], int x, int y, int w, int h, double radians);

void
CRenderSort::DrawObjects()
{
  struct Local {
    static inline bool CompareObjects(const Object& a, const Object& b) {
      return (a.sort_y < b.sort_y);
    }
  };

  // sort the object list
  std::stable_sort(
    m_objects.begin(),
    m_objects.end(),
    Local::CompareObjects
  );

  // draw images to screen
  for (unsigned i = 0; i < m_objects.size(); i++) {
    Object& o = m_objects[i];

    int tx[4];
    int ty[4];
    tx[0] = o.draw_x;
    ty[0] = o.draw_y;
    tx[1] = o.draw_x + o.draw_w - 1;
    ty[1] = o.draw_y;
    tx[2] = o.draw_x + o.draw_w - 1;
    ty[2] = o.draw_y + o.draw_h - 1;
    tx[3] = o.draw_x;
    ty[3] = o.draw_y + o.draw_h - 1;

    if (o.is_angled) {
      CalculateRotateBlitPoints(tx, ty, o.draw_x, o.draw_y, o.draw_w, o.draw_h, o.angle);
    }

    if (o.mask == CreateRGBA(255, 255, 255, 255)) {
      TransformBlitImage(o.image, tx, ty);
    } else {
      TransformBlitImageMask(o.image, tx, ty, o.mask);
    }
  }

  m_objects.clear();
}

////////////////////////////////////////////////////////////////////////////////
