/*
DO NOT, and I repeat, DO NOT look at this code! It has been tested on lab animals and
well... if I told you what happend PETA would be on me faster than dogs. Or bees. OR 
DOGS WITH BEES IN THEIR MOUTHS!

It has been uncommented FOR YOUR PROTECTION.

- SDHawk

I've since made it more bearable.

- aegis

*/

EvaluateScript("Particle.js")
EvaluateScript("Quick.js")
EvaluateScript("ParticleDefine.js")
EvaluateScript("Icon.js")
EvaluateScript("Normal.js")

EvaluateSystemScript("colors.js")

particleimg = LoadImage("small_particle.png")
sphereimg = LoadImage("sphere2.png")

font = GetSystemFont()
version = "Sphere " + GetVersionString()

list = GetGameList()
if (list.length == 0) { Abort("No games detected!") }

LoadIcons()

particles = particle_SPHERE()

var music = LoadSound("intro.mp3")
music.play(true)
if (Random(0,10) == 5) {
	switch (Random(0, 1)) {
		case 0: music.setPitch(2);  break;
		case 1: music.setPitch(.5); break;
  }
}


var credits = [
	"Sphere... Chad Austin",
	"Game menu... SDHawk",
	"Music... Chairface Chippendale"];

function game()
{
    SetFrameRate(60)

    font2 = GetSystemFont()
    var timer = 0;
    var count = 0;
    var inout = 1; // 1=out 0=in
    var alpha = 255;
    while (!IsAnyKeyPressed()) {
	    timer++;

	    switch (timer) {
		    case 500: ChangeParticlesSpeed(.5); break;
		    case 1000: ChangeParticlesSpeed(0); break;
		    case 1500: ChangeParticlesSpeed(.5); break;
		    case 2000: ChangeParticlesSpeed(1); timer = 0; break;
	    }
    		
	    if (inout == 1) {
	        alpha -= 2;
	    } else {
	        if (inout == 0) {
	            alpha += 2;
	        }
	    }
	    if (alpha <= 1) {
	        inout = 0;
	        alpha = 2;
	        count++;
	        if (count == credits.length) {
	            count = 0;
	        }
	    } else {
	        if (alpha >= 255) {
	            inout = 1;
	        }
	    }
	    font2.setColorMask(CreateColor(255,255,255,alpha))	

	    sphereimg.blit(0,0)
    		
	    ProcParticles()	    RenderParticles()
	    font.drawText(0,0,version)

	    font2.drawText(90,160,credits[count])
	    font.drawText(40,220,"ENTER to continue, Q for quick mode")
    		
	    FlipScreen()
    }
	
    ChangeParticlesSpeed(1);
    switch (GetKey()) {
        case KEY_Q: QuickMenu(); break;
        case KEY_ESCAPE: break;
        default: FadeOut(); Menu(); break;
    }
}

function Random(hi, lo) {
	return lo += Math.floor(Math.random() * (hi - lo));
}

function wait(time) {
  var start = GetTime();
  while (GetTime() < start + time) {
  }
}
