#include <algorithm>
#include "rendersort.hpp"


////////////////////////////////////////////////////////////////////////////////

void
CRenderSort::AddObject(int draw_x, int draw_y, int sort_y, int draw_w, int draw_h, IMAGE image, RGBA mask)
{
  Object object;
  object.draw_x = draw_x;
  object.draw_y = draw_y;
  object.sort_y = sort_y;
  object.draw_w = draw_w;
  object.draw_h = draw_h;
  object.image  = image;
  object.mask   = mask;
  m_objects.push_back(object);
}

////////////////////////////////////////////////////////////////////////////////

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

    int x[4];
    int y[4];
    x[0] = o.draw_x;
    y[0] = o.draw_y;
    x[1] = o.draw_x + o.draw_w - 1;
    y[1] = o.draw_y;
    x[2] = o.draw_x + o.draw_w - 1;
    y[2] = o.draw_y + o.draw_h - 1;
    x[3] = o.draw_x;
    y[3] = o.draw_y + o.draw_h - 1;
    if (o.mask == CreateRGBA(255, 255, 255, 255)) {
      TransformBlitImage(o.image, x, y);
    } else {
      TransformBlitImageMask(o.image, x, y, o.mask);
    }
  }

  m_objects.clear();
}

////////////////////////////////////////////////////////////////////////////////
