<?
  include("utility.inc.php");
  start();
  
  if( isset( $U_ACT ) ){
    switch( $U_ACT ){
      case "about":
        echo "this description of sphere is taken from aegisknight.org<br /><br />";
        gen_item( "overview", "Sphere is a 2D RPG engine. It allows people to create role-playing games like Final Fantasy VI or Phantasy Star." );
        gen_item( "graphics", "Internally, Sphere uses 32-bit color... That's 16.7 million colors and 256 levels of translucency on every image. It can use DirectDraw, GDI, or OpenGL to render to the screen. Sphere can also load PNG, JPEG, PCX, and BMP images." );
        gen_item( "sound", "Since Sphere uses <a href=\"http://aegisknight.org/audierer\">Audiere</a> for sound playback, it supports MP3, Ogg Vorbis, WAV, XM, IT. S3M, and MOD." );
        gen_item( "input", "Keyboard. Mouse. Joystick. What else is there to support? :)" );
        gen_item( "scripting", "Sphere uses <a href=\"http://mozilla.org/js/spidermonkey/\">SpiderMonkey</a> (<a href=\"http://mozilla.org\">Mozilla</a>'s <a href=\"http://mozilla.org/js\">JavaScript</a> implementation) for scripting. JavaScript is a very powerful, easy, and flexible language." );
        gen_item( "engine", "The core Sphere engine is just one executable. However, it can run multiple games. Each game is in a subdirectory of the executable. This means that the games don't have to be distributed with the engine, saving download time and disk space, and if Sphere is ported to another platform, such as Linux, the games will work exactly the same way.<br /><br />The Sphere engine is based around maps. Maps use tilesets, which are collections of tiles. A map of a beach would have tiles that represent the sand, the water, clouds, signs, and anything else on the beach. Sphere has support for animated tiles. Torches could flicker in a cave, or waves could lap at the shore. Maps support unlimited layers, each with parallax and automatic scrolling.<br /><br />Objects in Sphere are represented as entities. Townspeople are defined with spritesets, which are collections of frames. People can walk in eight directions, and have special directions (usually used for emotions or running). Unlike most RPG engines, spritesets in Sphere have a variable size. Large animal spritesets are used just as easily as people." );
        gen_item( "editor", "Sphere comes with an integrated environment for editing maps, spritesets, scripts, fonts, etc. You can even play your game's music and sound effects while you work." );
        
      case "current":
        gen_item( "recent release", "<a href=\"http://aegisknight.org/download/sphere-0.95a.zip\">Sphere 0.95 (alpha)</a>" );
        gen_item( "older releases", "<a href=\"http://aegisknight.org/download/sphere-0.94a.zip\">Sphere 0.94 (alpha)</a><br />
          <a href=\"http://aegisknight.org/download/sphere-0.93a.zip\">Sphere 0.93 (alpha)</a><br />
          <a href=\"http://aegisknight.org/download/sphere-0.93a-no-games.zip\">Sphere 0.93 (alpha, no games included)</a><br />
          <a href=\"http://aegisknight.org/download/sphere-0.92.zip\">Sphere 0.92</a><br />
          " );
    }
  }
  
  important_links();
  
  conclude();
?>