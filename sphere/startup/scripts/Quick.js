function QuickMenu()
{
    SetFrameRate(0);
    
	sphereimg.blit(0,0)
	ProcParticles()	RenderParticles()
	font.drawText(0,0,version)
	screen = GrabImage(0, 0, GetScreenWidth(), GetScreenHeight())
	
	SetFrameRate(60);
	
	font.setColorMask(White)
	window = LoadWindowStyle("window.rws")
	swindow = LoadWindowStyle("swindow.rws")
	var selection = 0
	var scroll = 0
	var ico = LoadImage(icon[selection].file)
	while (1) 
	{
		screen.blit(0,0);
		window.drawWindow(50,10,font.getStringWidth(list[selection].name),font.getHeight())
		window.drawWindow(50,30,font.getStringWidth(list[selection].author),font.getHeight())
		window.drawWindow(50, 50, 200, 100)
		window.drawWindow(270,80,25,25)
		window.drawWindow(20,160,280,70)
		
		font.drawText(50,10,list[selection].name)
		font.drawText(50,30,list[selection].author)
		font.drawTextBox(20, 160, 280, 70, 0, list[selection].description)
		count = 0
		if (list.length <= 9)
			{
			while (count <= list.length-1)
				{
					font.drawText(50,50+count*10,list[count+scroll].name)
					count++;
				}
			}
		else 
			{
				while (count <= 9)
					{
						font.drawText(50,50+count*10,list[count+scroll].name)
						count++;
					}
			}
		count = 0

		if (selection >= 9) { swindow.drawWindow(50,140,font.getStringWidth(list[selection].name),font.getHeight()) }
		else { swindow.drawWindow(50,50+selection*10,font.getStringWidth(list[selection].name),font.getHeight()) }

		ico.blit(270,80)
		FlipScreen();
		
		while (AreKeysLeft()) {
			switch (GetKey()) {
				case KEY_DOWN: if (selection <= list.length-2) { selection++; if (selection >= 10) { scroll++; } } ico = LoadImage(icon[selection].file)
				break; 
				case KEY_UP: if (selection > 0) { if (selection >= 10) { scroll--; } selection--; } ico = LoadImage(icon[selection].file)
				break;
				case KEY_ENTER: ExecuteGame(list[selection].directory);
				break;
				case KEY_ESCAPE: Exit()
				break
				}
			}

		//if (selection >= 10)
		//	 {
		//	 scroll = list.length-selection
		//	 }
		//else { scroll = 0 }

		//if (IsKeyPressed(KEY_DOWN) && selection <= list.length-2) {  wait(50); }
		//if (IsKeyPressed(KEY_UP) && selection > 0) { selection--; wait(50); } 
	}
}