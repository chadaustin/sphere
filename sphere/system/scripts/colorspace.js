function DegreesToRadians(degrees) {
  return degrees * Math.PI / 180.0;
}

function RadiansToDegrees(radians) {
  return radians * 180.0 / Math.PI;
}

function CreateRGBA(red, green, blue, alpha) {
  return new RGBA(red, green, blue, alpha);
}

function CreateHSIA(hue, saturation, intensity, alpha) {
  return new HSIA(hue, saturation, intensity, alpha);
}


function Clamp(number, low, high) {
  if (number > high)   number = high;
  if (number < low) number = low;
  return number;
}

/*********************************************************/

function SphereColorToColor(c) {
  return RGBAToHSIA(new RGBA(c.red, c.green, c.blue, c.alpha));
}

function ColorToSphereColor(c) {
  return CreateColor(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());
}

/*********************************************************/

function HSIA(h, s, i, a) {
  this.hue = h;
  this.saturation = s;
  this.intensity = i;
  this.alpha = a;

  this.getRed = function() {
    var tmp = pHSIAtoRGBA(this.hue, this.saturation, this.intensity, this.alpha);
    return tmp.getRed();
  }

  this.setRed = function(value) {
    var tmp = pHSIAtoRGBA(this.hue, this.saturation, this.intensity, this.alpha);
    tmp.setRed(value);
    tmp = RGBAToHSIA(tmp);
    this.hue = tmp.hue;
    this.saturation = tmp.saturation;
    this.intensity = tmp.intensity;
    this.alpha = tmp.alpha;
  }

  this.getGreen = function() {
    var tmp = pHSIAtoRGBA(this.hue, this.saturation, this.intensity, this.alpha);
    return tmp.getGreen();
  }

  this.setGreen = function(value) {
    var tmp = pHSIAtoRGBA(this.hue, this.saturation, this.intensity, this.alpha);
    tmp.setGreen(value);
    tmp = RGBAToHSIA(tmp);
    this.hue = tmp.hue;
    this.saturation = tmp.saturation;
    this.intensity = tmp.intensity;
    this.alpha = tmp.alpha;
  }

  this.getBlue = function() {
    var tmp = pHSIAtoRGBA(this.hue, this.saturation, this.intensity, this.alpha);
    return tmp.getBlue();
  }

  this.setBlue = function(value) {
    var tmp = pHSIAtoRGBA(this.hue, this.saturation, this.intensity, this.alpha);
    tmp.setBlue(value);
    tmp = RGBAToHSIA(tmp);
    this.hue = tmp.hue;
    this.saturation = tmp.saturation;
    this.intensity = tmp.intensity;
    this.alpha = tmp.alpha;
  }

  this.getCyan = function() {
    var tmp = pHSIAtoRGBA(this.hue, this.saturation, this.intensity, this.alpha);
    return tmp.getCyan();
  }
  
  this.setCyan = function(value) {
    var tmp = pHSIAtoRGBA(this.hue, this.saturation, this.intensity, this.alpha);
    tmp.setCyan(value);
    tmp = RGBAToHSIA(tmp);
    this.hue = tmp.hue;
    this.saturation = tmp.saturation;
    this.intensity = tmp.intensity;
    this.alpha = tmp.alpha;
  }

  this.getMagenta = function() {
    var tmp = pHSIAtoRGBA(this.hue, this.saturation, this.intensity, this.alpha);
    return tmp.getMagenta();
  }

  this.setMagenta = function(value) {
    var tmp = pHSIAtoRGBA(this.hue, this.saturation, this.intensity, this.alpha);
    tmp.setMagenta(value);
    tmp = RGBAToHSIA(tmp);
    this.hue = tmp.hue;
    this.saturation = tmp.saturation;
    this.intensity = tmp.intensity;
    this.alpha = tmp.alpha;
  }

  this.getYellow = function() {
    var tmp = pHSIAtoRGBA(this.hue, this.saturation, this.intensity, this.alpha);
    return tmp.getYellow();
  }

  this.setYellow = function(value) {
    var tmp = pHSIAtoRGBA(this.hue, this.saturation, this.intensity, this.alpha);
    tmp.setYellow(value);
    tmp = RGBAToHSIA(tmp);
    this.hue = tmp.hue;
    this.saturation = tmp.saturation;
    this.intensity = tmp.intensity;
    this.alpha = tmp.alpha;
  }

  this.getHue = function() {
    return this.hue;
  }

  this.setHue = function(value) {
    this.hue = value;
    // this.hue = Clamp(this.hue, 0, DegreesToRadians(360));
  }

  this.getSaturation = function() {
    return this.saturation;
  }

  this.setSaturation = function(value) {
    this.saturation = value;
    // this.saturation = Clamp(this.saturation, 0, 1);
  }

  this.getIntensity = function() {
    return this.intensity;
  }

  this.setIntensity = function(value) {
    this.intensity = value;
    // this.intensity = Clamp(this.intensity, 0, 1);
  }

  this.getAlpha = function() {
    return this.alpha;
  }

  this.setAlpha = function(value) {
    this.alpha = alpha;
  }

}

/*********************************************************/

function RGBA(r, g, b, a) {
  this.red = r;
  this.green = g;
  this.blue = b;
  this.alpha = a;

  var max = 255;

  this.getRed = function() {
    return this.red;
  }

  this.setRed = function(value) {
    this.red = value;
  }

  this.getGreen = function() {
    return this.green;
  }

  this.setGreen = function(value) {
    this.green = value;
  }

  this.getBlue = function() {
    return this.blue;
  }

  this.setBlue = function(value) {
    this.blue = value;
  }

  this.getHue = function() {
    var tmp = pRGBAtoHSIA(this.red, this.green, this.blue, this.alpha);
    return tmp.getHue();
  }

  this.setHue = function(value) {
    var tmp = pRGBAtoHSIA(this.red, this.green, this.blue, this.alpha);
    tmp.hue = value;
    tmp = HSIAToRGBA(tmp);
    this.red = tmp.red;
    this.green = tmp.green;
    this.blue = tmp.blue;
    this.alpha = tmp.alpha;
  }

  this.getSaturation = function() {
    var tmp = pRGBAtoHSIA(this.red, this.green, this.blue, this.alpha);
    return tmp.getSaturation();
  }

  this.setSaturation = function(value) {
    var tmp = pRGBAtoHSIA(this.red, this.green, this.blue, this.alpha);
    tmp.setSaturation(value);
    tmp = HSIAToRGBA(tmp);
    this.red = tmp.red;
    this.green = tmp.green;
    this.blue = tmp.blue;
    this.alpha = tmp.alpha;
  }

  this.getIntensity = function() {
    var tmp = pRGBAtoHSIA(this.red, this.green, this.blue, this.alpha);
    return tmp.getIntensity();
  }

  this.setIntensity = function(value) {
    var tmp = pRGBAtoHSIA(this.red, this.green, this.blue, this.alpha);
    tmp.setIntensity(value);
    tmp = HSIAToRGBA(tmp);
    this.red = tmp.red;
    this.green = tmp.green;
    this.blue = tmp.blue;
    this.alpha = tmp.alpha;
  }

  this.getCyan = function() {
    return max - this.red;
  }

  this.getMagenta = function() {
    return max - this.green;
  }

  this.getYellow = function() {
    return max - this.blue;
  }

  this.setCyan = function(value) {
    this.red = max - value;
  }

  this.setMagenta = function(value) {
    this.green = max - value;
  }

  this.setYellow = function(value) {
    this.blue = max - value;
  }

  this.getAlpha = function() {
    return this.alpha;
  }

  this.setAlpha = function(value) {
    this.alpha = alpha;
  }
}

/*********************************************************/


function pRGBAtoHSIA(red, green, blue, alpha) {
  var p = CreateHSIA(0, 0, 0, alpha);

  var max = 255.0;
  var r = ((1.0/max) * red);
  var g = ((1.0/max) * green);
  var b = ((1.0/max) * blue);

  var total = (r + g + b);
  var third_total = ((total > 0.0) ? (total / 3.0) : 0.0);
  var three_over_total = ((total > 0.0) ? (3.0 / total) : 0.0);

  var min = Math.min(r, Math.min(g, b));

  var htop = 0.5 * ((r - g) + (r - b));
  var hbottom = Math.sqrt((Math.pow((r-g), 2) + ((r - b) * (g-b))));

  p.intensity = third_total;
  p.saturation = 1.0 - three_over_total * min;
  p.hue = Math.acos(htop / (hbottom > 0 ? hbottom : 1.0));

  if (b > g) p.hue = DegreesToRadians(360) - p.hue;

  return p;
}

function RGBAToHSIA(rgba) {
  return pRGBAtoHSIA(rgba.red, rgba.green, rgba.blue, rgba.alpha);
}


function pHSIAtoRGBA(hue, saturation, intensity, alpha) {
  var max = 255;
  var red = 0
  var green = 0;
  var blue = 0;

  var hue_in_degrees = RadiansToDegrees(hue);

  if (hue_in_degrees >= 0 && hue_in_degrees <= 120) {
    blue  = intensity * (1.0 - saturation);
    red   = intensity * (1.0 + (saturation * Math.cos(hue)) / (Math.cos(DegreesToRadians(60) - hue)));
    green = (3.0 * intensity) * (1.0 - ((red + blue) / (3.0 * intensity)));
  }
  else
  if (hue_in_degrees > 120 && hue_in_degrees <= 240) {
    hue = hue - DegreesToRadians(120);
    green = intensity * (1.0 + (saturation * Math.cos(hue)) / (Math.cos(DegreesToRadians(60) - hue)));
    red   = intensity * (1.0 - saturation);
    blue  = (3.0 * intensity) * (1.0 - ((red + green) / (3.0 * intensity)));
  }
  else
  if (hue_in_degrees > 240 && hue_in_degrees <= 360)
  {
    hue = hue - DegreesToRadians(240);
    blue  = intensity * (1.0 + (saturation * Math.cos(hue)) / (Math.cos(DegreesToRadians(60) - hue)));
    green = intensity * (1.0 - saturation);
    red   = (3.0 * intensity) * (1.0 - ((green + blue) / (3.0 * intensity)));
  }

  /* denormalize */
  red *= max;
  green *= max;
  blue *= max;

  return CreateRGBA(Math.round(red), Math.round(green), Math.round(blue), alpha);
}

function HSIAToRGBA(hsia) {
  return pHSIAtoRGBA(hsia.hue, hsia.saturation, hsia.intensity, hsia.alpha);
}

/*********************************************************/

function ColorToString(c) {
  return "red " + c.getRed() + " green " + c.getGreen() + " blue " + c.getBlue() + " alpha " + c.getAlpha() +
         " cyan " + c.getCyan() + " magenta " + c.getMagenta() + " yellow " + c.getYellow() +
         " hue = " + c.getHue() + " saturation " + c.getSaturation() + " intensity " + c.getIntensity();
}

/*********************************************************/