EvaluateSystemScript("colors.js");
EvaluateSystemScript("time.js");
EvaluateSystemScript("screen.js");

function Menu()
{
  if (this instanceof Menu == false) {
    return new Menu();
  }

  // default properties
  this.font            = GetSystemFont()
  this.window_style    = GetSystemWindowStyle()
  this.arrow           = GetSystemArrow();
  this.up_arrow        = GetSystemUpArrow();
  this.down_arrow      = GetSystemDownArrow();
  this.mid_arrow       = LoadImage("mid.png");
  this.top_fill        = LoadImage("top.png");
  this.mid_fill        = LoadImage("fil.png");
  this.bot_fill        = LoadImage("bot.png");
  this.sel_color       = CreateColor(74, 117, 187, 128);
  this.escape_function = function() { }
  this.items = new Array();
}

// add item
Menu.prototype.addItem = function(name, callback, color) {

  if (color == undefined) {
    color = White;
  }

  var item = new Object;
  item.name     = name;
  item.callback = callback;
  item.color    = color;
  this.items[this.items.length] = item;
}

// execute
Menu.prototype.execute = function(x, y, w, h) {
  with (this) {
    var background = GrabImage(0, 0, GetScreenWidth(), GetScreenHeight());
    hi=0; wi = 0;
    while(hi != h && wi != w){
      background.blit(0,0);
      window_style.drawWindow(GetScreenWidth()/2-wi/2, GetScreenHeight()/2-hi/2, wi, hi);
	if(hi < h) hi += 20;
	if(wi < w) wi += 20;
	if(hi > h) hi = h;
	if(wi > w) wi = w;
      Delay(1000/60);
      FlipScreen();
      
    }
    

    var text_height = font.getHeight();
    var shown_items = Math.floor(h / text_height);

    var selection = 0;
    var top_selection = 0;

    while (true) {
      // draw background
      background.blit(0, 0);

      // draw the window
      window_style.drawWindow(x, y, w, h);

      // draw the menu items
      for (var i = 0; i < shown_items; i++) {
        if (i < items.length) {
          font.setColorMask(Black);
          font.drawText(x + 2,  y + i * text_height + 2, items[i + top_selection].name);
          font.setColorMask(items[i + top_selection].color);
          font.drawText(x + 1,     y + i * text_height + 1,     items[i + top_selection].name);
        }
      }

      // draw the selection arrow
      //arrow.blit(x, y + (selection - top_selection) * text_height);
        ya = y + (selection - top_selection) * text_height;
        right = x + font.getStringWidth(items[selection].name);
        Line(x + 1,     ya + 1,                     right + 2, ya + 1,                    sel_color);
        Line(right + 2, ya + 1,                     right + 2, ya + font.getHeight() + 1, sel_color);
        Line(right + 2, ya + font.getHeight() + 1,  x + 1,     ya + font.getHeight() + 1, sel_color);
        Line(x + 1,     ya + font.getHeight() + 1,  x + 1,     ya + 1,                    sel_color);                    
      // draw the scroll bar

        up_arrow.blit(x + w - up_arrow.width, y);
        down_arrow.blit(x + w - down_arrow.width, y + h - down_arrow.height);
        mid_arrow.transformBlit(x + w - mid_arrow.width, y + up_arrow.height,
                                x + w,                   y + up_arrow.height,
                                x + w,                   y + h - down_arrow.height,
                                x + w - mid_arrow.width, y + h - down_arrow.height);
        fh = font.getHeight();
        mh = h - up_arrow.height - down_arrow.height;
        tot = items.length;
        top = mh / tot * top_selection;
	
	bot = top + (mh / tot) * shown_items;
	if(tot <= shown_items) bot = GetScreenHeight() - 16 - up_arrow.height - y - down_arrow.height; // a hack because I am lazy.
        
        mid_fill.transformBlit(x + w - mid_arrow.width + 1, y + up_arrow.height + top + 1,
                               x + w - 1,                   y + up_arrow.height + top + 1,
                               x + w - 1,                   y + up_arrow.height + bot - 2,
                               x + w - mid_arrow.width + 1, y + up_arrow.height + bot - 1);
        top_fill.blit(x + w - mid_arrow.width+1, y + up_arrow.height + top);
        bot_fill.blit(x + w - mid_arrow.width+1, y + up_arrow.height + bot - 1);    
      FlipScreen();

      // handle keypresses
      while (AreKeysLeft()) {
        switch (GetKey()) {
          case KEY_ENTER: {
            var item = items[selection];
            hi=h;
            for(wi=w; wi>0; wi-=GetScreenWidth()*(10/320)){
              background.blit(0,0);
              window_style.drawWindow(GetScreenWidth()/2-wi/2, GetScreenHeight()/2-hi/2, wi, hi);
              if(hi>0) hi-=GetScreenWidth()*(10/320);
              Delay(1000/60);
              FlipScreen();
            }
            FadeOut(500);
            item.callback();
            return;
          }

          case KEY_ESCAPE: {
            escape_function();
            return;
          }

          case KEY_DOWN: {
            if (selection < items.length - 1) {
              selection++;
              if (selection >= top_selection + shown_items) {
                top_selection++;
              }
            }
            break;
          }

          case KEY_UP: {
            if (selection > 0) {
              selection--;
              if (selection < top_selection) {
                top_selection--;
              }
            }
            break;
          }

        }
      } // end handle input
    }

  } // end with
}
