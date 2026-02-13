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

void creditClick(int mx, int my) {
	if (mx >= 388 && mx <= 578 && my >= 80 && my <= 130) {
		gameState = 2;
	}
}

void settingClick(int mx, int my){
	if (mx >= 918 && mx <= 960 && my >= 518 && my <= 560) {
		gameState = 3;

void howToPlayClick(int mx, int my) {
	if (mx >= 38 && mx <= 110 && my >= 36 && my <= 90) {
		gameState = 4;
	}
}

#endif