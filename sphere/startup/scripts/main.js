EvaluateSystemScript("screen.js");
EvaluateSystemScript("audio.js");
EvaluateScript("menu.js");
EvaluateSystemScript("colors.js");


function game()
{
  intro();
  main_menu();
}


function intro()
{
  var image = LoadSurface("intro.jpeg");
  var sound = LoadSound("intro.wav");
  image.rescale(GetScreenWidth(), GetScreenHeight());
  image = image.createImage();

  sound.play(false);
  
  image.blit(0, 0);
  FadeIn(500);
  
  while (!AreKeysLeft()) {
    image.blit(0, 0);
    FlipScreen();
  }
  GetKey();
  
  image.blit(0, 0);
  ApplyColorMask(Black);  // FadeOut() leaves the backbuffer in place
  
  sound.stop();
}


function main_menu()
{
  var unactive = CreateColor(200,200,200); // <- Colour of not selected
  while (AreKeysLeft()){
	GetKey();
  }
  var image = LoadSurface("intro.jpeg");
  var sound = LoadSound("intro.wav");
  image.rescale(GetScreenWidth(), GetScreenHeight());
  image = image.createImage();
  
  var font         = LoadFont("base.rfn");
  var window_style = LoadWindowStyle("base.rws");
  var arrow        = GetSystemArrow();
  var up_arrow     = GetSystemUpArrow();
  var down_arrow   = GetSystemDownArrow();
  
  // load the game list
  var game_list = GetGameList();
  if (game_list.length == 0) {
    font.drawText(0, 0, "No games detected");
    FlipScreen();
    GetKey();
    return;
  }
  
  // create the menu object
  var menu = new Menu();
  menu.window_style=window_style;
  menu.escape_function = Exit;
  menu.font = font;
  menu.up_arrow = LoadImage("up.png");
  menu.down_arrow = LoadImage("down.png");
  // add all of the games to the object
  for (var i = 0; i < game_list.length; i++) {
    var o = game_list[i];
    menu.addItem(o.name, new Function("ExecuteGame('" + o.directory + "')"),unactive); // <-
  }

  while (true) {
    image.blit(0, 0);
    font.drawText(9, GetScreenHeight()-14, "Sphere " + GetVersionString());
    font.setColorMask(White);
    font.drawText(GetScreenWidth()-9-font.getStringWidth("Chad Austin (c) 1997-2003"), GetScreenHeight()-14, "Chad Austin (c) 1997-2001");
    menu.execute(16, 16, GetScreenWidth() - 32, GetScreenHeight() - 32);
  }
  
}
