#ifndef HOWTOPLAYPAGE_H
#define HOWTOPLAYPAGE_H

#include "iGraphics.h"

extern int gameState;
static unsigned int howToPlayImg = 0;

void drawHowToPlayPage()
{
	if (howToPlayImg == 0) {
		howToPlayImg = iLoadImage("Image//How To Play.png");
	}
	iShowImage(0, 0, 1000, 600, howToPlayImg);

	// BACK BUTTON
	iSetColor(200, 0, 0);
	iFilledRectangle(20, 540, 80, 40);
	iSetColor(255, 255, 255);
	iText(20 + 80 / 2 - (4 * 9) / 2, 540 + 40 / 2 - 15 / 2, "BACK", (void*)0x0002);
}

void howToPlayPageClick(int mx, int my) {
	// Back button click area
	if (mx >= 20 && mx <= 100 && my >= 540 && my <= 580) {
		gameState = 0; // Return to Main Menu (Home Page)
	}
}

#endif
