var scroll = 0
var windowo = 0
var focus = 0
var timer = 0
var gam = 0

function Menu()
{
    font.setColorMask(White)
    particleimg = LoadImage("big_particle.png")
    particle_MENU()

    var mouseimg = LoadImage("mouse.png")
    var x = 0
    var y = 0
    var alpha = -65
    var ud = 0
    SetFrameRate(100)
    img = LoadIconImgs()
    window = LoadWindowStyle("window.rws")
    ChangeParticlesColor(CreateColor(0,0,255,alpha))
    font.setColorMask(CreateColor(255,255,255,alpha))
    while (1) {
		timer++;

		if (timer > 4000 && music.getVolume() != 0) {
		    music.setVolume(music.getVolume()-1)
		}

		switch (timer) {
			case 1000: ChangeParticlesSpeed(0); break;
			case 3000: ChangeParticlesSpeed(.5); timer = 0;	break;
			case 4001: ChangeParticlesSpeed(1);	break;
			case 4300: ExecuteGame(list[gam].directory); break;
		}
	
	
		sphereimg.blit(0,0)
		x = GetMouseX()
		y = GetMouseY()
		ProcParticles()		RenderParticles()
		RenderIcons(alpha)
		RenderText()
		font.drawText(0,0,"Left/right arrow to navigate")
		font.drawText(0,10,"Left mouse button to run")
		mouseimg.blit(x,y)
		
		if (windowo == 1) {
			window.drawWindow(20,100,font.getStringWidth(list[focus].name),font.getHeight())
			window.drawWindow(20,120,font.getStringWidth(list[focus].author),font.getHeight())
			window.drawWindow(20,160,280,70)
			
			font.drawText(20,100,list[focus].name)
			font.drawText(20,120,list[focus].author)
			font.drawTextBox(20, 160, 280, 70, 0, list[focus].description)
			
			windowo = 0
		}
		FlipScreen()
		
		if (ud == 0) alpha--
		if (ud == 1) alpha++
		if (alpha >= 235) { ud = 0 }
		if (alpha < 140) { ud = 1 }
		ChangeParticlesColor(CreateColor(0,0,255,alpha))
		font.setColorMask(CreateColor(255,255,255,alpha))
		CheckInput()
	}
}

function CheckInput()
{
    var count = 0
    focus = -5
    if (list.length > 7) {
	    while (count < particles.length) {
			if (GetMouseX() <= particles[count].x+45 && GetMouseX() >= particles[count].x-10 && +
				GetMouseY() <= particles[count].y+35 && GetMouseY() >= particles[count].y-10)
			{
			    particles[count].color = CreateColor(0,0,255,255); focus = count+scroll;
			}
			count++; 
		}
    } else {
	    while (count < list.length)	{
			if (GetMouseX() <= particles[count].x+45 && GetMouseX() >= particles[count].x-10 && +
				GetMouseY() <= particles[count].y+35 && GetMouseY() >= particles[count].y-10)
			{
			    particles[count].color = CreateColor(0,0,255,255); focus = count+scroll;
			}
			count++; 
		}
    }
    while (AreKeysLeft()) { 
    	switch (GetKey()) {
		    case KEY_LEFT: if (scroll <= list.length-9) scroll++; img = LoadIconImgs();	break; 
		    case KEY_RIGHT: if (scroll > 0) scroll--; img = LoadIconImgs();	break;
		    case KEY_ESCAPE: Exit(); break;
		}
	}
	
    if (IsMouseButtonPressed(MOUSE_LEFT) && focus != -5) {
        ScaterParticles2();
        timer = 4000;
        gam = focus;
    }
    if (focus != -5) {
		windowo = 1
	}
}

function RenderIcons(alpha) {
    if (list.length > 7) {
	    var count = 0
	    while (count < 8) {
			img[count].blitMask(particles[count].x+10,particles[count].y+7,CreateColor(255,255,255,alpha))
			count++; 
		}
	} else {
	    var count = 0
	    while (count < list.length)	{
			img[count].blitMask(particles[count].x+10,particles[count].y+7,CreateColor(255,255,255,alpha))
			count++; 
		}
	}
}

function RenderText()
{
    if (list.length > 7) {
	    var count = 0
	    while (count < 8) {
			font.drawText(particles[count].x-5,particles[count].y+35,list[count+scroll].directory)
			count++; 
		}
    } else {
	    var count = 0
	    while (count < list.length)	{
			font.drawText(particles[count].x-5,particles[count].y+35,list[count+scroll].directory)
			count++; 
		}
    }
}

function LoadIconImgs()
{
    if (list.length > 7) {
	    var count = 0
	    var img = new Array()
	    while (count < 8) {
			img[count] = LoadImage(icon[count+scroll].file)
			count++;
		}
	    return img
    } else {
	    var count = 0
	    var img = new Array()
	    while (count < list.length)	{
			img[count] = LoadImage(icon[count+scroll].file)
			count++;
		}
	    return img
    }
}