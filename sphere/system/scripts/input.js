function GetString(x, y, font)
{
  var background = GrabImage(0, 0, GetScreenWidth(), GetScreenHeight());
  
  var str = "";
  var cursor_position = 0;

  while (true) {
    
    background.blit(0, 0);
    font.drawText(x, y, str);
    font.drawText(x + font.getStringWidth(str.slice(0, cursor_position), cursor_position), y, "|");

    FlipScreen();
    
    while (AreKeysLeft()) {
    
      var key = GetKey();
      switch (key) {
      
        // done
        case KEY_ENTER: { 
          return str;
        }
        
        // backspace
        case KEY_BACKSPACE: {
          if (str != "") {
           str = str.slice(0, cursor_position - 1) + str.slice(cursor_position + 1);
          }
          if (cursor_position > 0)
            cursor_position -= 1;
          break;
        }

        case KEY_LEFT: {
           if (cursor_position > 0)
             cursor_position -= 1;
          break;
        }

        case KEY_RIGHT: {
          if (cursor_position <= str.length - 1)
            cursor_position += 1;
          break;
        }
        
        default: {
          var shift = IsKeyPressed(KEY_SHIFT);
          if (GetKeyString(key, shift) != "") {
            str = str.slice(0, cursor_position) + GetKeyString(key, shift) + str.slice(cursor_position);
            cursor_position += 1;
          }
        }
      } // end switch
      
    } // end while (keys left)

  } // end while (true)
}
