#ifndef MENU_H
#define MENU_H

extern int gameState;
int playHover = 0;
int creditHover = 0;
int settingHover = 0;
int helpHover = 0;

void drawMenu()
{
	iShowBMP(0, 0, "Image//homepage.bmp");
	
	// Play button image

	if (playHover)
		iShowBMP2(0, 0, "Image//play.bmp", 0);
	

	// Draw Credit button hover
	if (creditHover)
		iShowBMP2(0, 0, "Image//credit.bmp", 0);


	if (settingHover)
		iShowBMP2(0, 0, "Image//setting.bmp", 0);

	// HOW TO PLAY BUTTON
	if (helpHover)
		iShowBMP2(0, 0, "Image//how_to_play.bmp", 0);
	

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

void creditClick(int mx, int my) {
	if (mx >= 388 && mx <= 578 && my >= 80 && my <= 130) {
		gameState = 2;
	}
}

void settingClick(int mx, int my){
	if (mx >= 918 && mx <= 960 && my >= 518 && my <= 560) {
		gameState = 3;
	}
}

void howToPlayClick(int mx, int my) {
	if (mx >= 38 && mx <= 110 && my >= 36 && my <= 90) {
		gameState = 4;
	}
}

#endif