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

void CalculateRotateBlitPoints(int tx[4], int ty[4], double x, double y, double w, double h, double radians);

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
    tx[1] = o.draw_x + o.draw_w /*- 1*/;
    ty[1] = o.draw_y;
    tx[2] = o.draw_x + o.draw_w /*- 1*/;
    ty[2] = o.draw_y + o.draw_h /*- 1*/;
    tx[3] = o.draw_x;
    ty[3] = o.draw_y + o.draw_h /* - 1*/;

    if (o.is_angled) {
      CalculateRotateBlitPoints(tx, ty, o.draw_x, o.draw_y, o.draw_w, o.draw_h, o.angle);
    }

    bool normal_blit = false;
    bool masked = (o.mask == CreateRGBA(255, 255, 255, 255));

    // BlitImage is usually faster than TransformBlit, so use BlitImage if possible
    if (tx[0] == tx[3] && tx[1] == tx[2] && ty[0] == ty[1] && ty[2] == ty[3]) {
      int dw = tx[2] - tx[0] /*+ 1*/;
      int dh = ty[2] - ty[0] /*+ 1*/;
      if (dw == GetImageWidth(o.image) && dh == GetImageHeight(o.image)) {
        normal_blit = true;
      }
    }

    if (normal_blit) {
      if (masked) {
        BlitImage(o.image, tx[0], ty[0]);
      } else {
        BlitImageMask(o.image, tx[0], ty[0], o.mask);
      }
    } else {
      if (masked) {
        TransformBlitImage(o.image, tx, ty);
      } else {
        TransformBlitImageMask(o.image, tx, ty, o.mask);
      }
    }
  }

  m_objects.clear();
}

////////////////////////////////////////////////////////////////////////////////
