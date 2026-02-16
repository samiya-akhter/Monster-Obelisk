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

// Detect press and release
void creditMouse(int button, int state, int mx, int my)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			if (mx >= backX && mx <= backX + backWidth &&
				my >= backY && my <= backY + backHeight)
				backPressed = 1;
		}
		else if (state == GLUT_UP)
		{
			if (backPressed && mx >= backX && mx <= backX + backWidth &&
				my >= backY && my <= backY + backHeight)
				gameState = 0; // Go back to menu

			backPressed = 0;
		}
	}
}

#endif
