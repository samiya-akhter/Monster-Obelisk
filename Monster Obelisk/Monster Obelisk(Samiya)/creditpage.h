#ifndef CREDITPAGE_H
#define CREDITPAGE_H

int backHover = 0;
int backPressed = 0;

// Button coordinates
int backX = 427, backY = 57;
int backWidth = 131, backHeight = 37;

void drawCreditPage()
{
	// Draw background
	iShowImage(0, 0,1000,600,iLoadImage( "image//creditpage.png"));

	// Draw back button only if hovered or pressed
	if (backHover || backPressed)
		iShowBMP2(0, 0, "image//back.bmp",0);
}

// Detect hover
void creditPassiveMouseMove(int mx, int my)
{
	if (mx >= backX && mx <= backX + backWidth &&
		my >= backY && my <= backY + backHeight)
		backHover = 1;
	else
		backHover = 0;
}

void backClick(int mx, int my) {
	if (mx >= 425 && mx <=560 && my>= 55 && my <= 94) {
		gameState = 0;
	}
}
#endif
