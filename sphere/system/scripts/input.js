function GetString(x, y, font)
{
  var background = GrabImage(0, 0, GetScreenWidth(), GetScreenHeight());
  
  var str = "";
  
  while (true) {
    
    background.blit(0, 0);
    font.drawText(x, y, str + "_");
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
            str = str.substring(0, str.length - 1);
          }
        }
        
        default: {
          str += GetKeyString(key, IsKeyPressed(KEY_SHIFT));
        }
      } // end switch
      
    } // end while (keys left)

  } // end while (true)
}
