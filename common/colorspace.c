// colorspace conversion routines
// algorithms taken from http://blas.cis.mcmaster.ca/~monger/hsl-rgb.html


#include "colorspace.h"


////////////////////////////////////////////////////////////////////////////////

static double _Min(double a, double b)
{
  return (a < b ? a : b);
}

////////////////////////////////////////////////////////////////////////////////

static double _Max(double a, double b)
{
  return (a > b ? a : b);
}

////////////////////////////////////////////////////////////////////////////////

HSL RGBtoHSL(RGB rgb)
{
  double red   = rgb.red   / 255.0;
  double green = rgb.green / 255.0;
  double blue  = rgb.blue  / 255.0;

  double mincolor = _Min(red, _Min(green, blue));
  double maxcolor = _Max(red, _Max(green, blue));

  double hue;
  double saturation;
  double luminance;

  HSL hsl;

  luminance = (maxcolor + mincolor) / 2;

  if (maxcolor == mincolor)
  {
    hue = 0;
    saturation = 0;
  }
  else
  {
    if (luminance < 0.5)
      saturation = (maxcolor - mincolor) / (maxcolor + mincolor);
    else
      saturation = (maxcolor - mincolor) / (2.0 - maxcolor - mincolor);

    if (red == maxcolor)
      hue = (green - blue) / (maxcolor - mincolor);
    else if (green == maxcolor)
      hue = 2.0 + (blue - red) / (maxcolor - mincolor);
    else if (blue == maxcolor)
      hue = 4.0 + (red - green) / (maxcolor - mincolor);
  }

  if (hue < 0.0)
    hue += 6.0;

  hsl.hue        = (byte)((hue / 6.0) * 255);
  hsl.saturation = (byte)(saturation  * 255);
  hsl.luminance  = (byte)(luminance   * 255);
  return hsl;
}

////////////////////////////////////////////////////////////////////////////////

static double ftemp(double temp1, double temp2, double temp3)
{
  if (temp3 < 0.0)
    temp3 += 1.0;
  else if (temp3 > 1.0)
    temp3 -= 1.0;

  if (6.0 * temp3 < 1.0)
    return temp1 + (temp2 - temp1) * 6.0 * temp3;
  else if (2.0 * temp3 < 1.0)
    return temp2;
  else if (3.0 * temp3 < 2.0)
    return temp1 + (temp2 - temp1) * ((2.0 / 3.0) - temp3) * 6.0;
  else
    return temp1;
}

////////////////////////////////////////////////////////////////////////////////

RGB HSLtoRGB(HSL hsl)
{
  double hue = hsl.hue        / 255.0;
  double sat = hsl.saturation / 255.0;
  double lum = hsl.luminance  / 255.0;

  double temp1;
  double temp2;

  double red;
  double green;
  double blue;

  RGB rgb;

  if (lum < 0.5)
    temp2 = lum * (1.0 + sat);
  else
    temp2 = lum + sat - (lum * sat);

  temp1 = 2.0 * lum - temp2;

  red   = ftemp(temp1, temp2, hue + 1.0 / 3.0);
  green = ftemp(temp1, temp2, hue);
  blue  = ftemp(temp1, temp2, hue - 1.0 / 3.0);

  rgb.red   = (byte)(red   * 255);
  rgb.green = (byte)(green * 255);
  rgb.blue  = (byte)(blue  * 255);
  return rgb;
}

////////////////////////////////////////////////////////////////////////////////
