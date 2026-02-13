#ifndef MENU_H
#define MENU_H

extern int gameState;

void drawMenu()
{
	iShowBMP(0, 0, "Image//homepage.bmp");
	
	// Play button image
	iShowBMP2(0, 0, "Image//play.bmp",0);
}

void playClick(int mx, int my)
{
	// PLAY button area
	if (mx >= 398 && mx <= 572 &&
		my >= 160 && my <= 215)
	{
		gameState = 1;   // Go to playpage
	}
}

#endif