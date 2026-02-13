#ifndef CREDITPAGE_H
#define CREDITPAGE_H

void drawCreditPage()
{
	iShowBMP(0, 0, "image//creditpage.bmp");
}

void backClick(int mx, int my) {
	if (mx >= 427 && mx <= 558 && my >= 57 && my <= 94) {
		gameState = 0;
	}
}

#endif
