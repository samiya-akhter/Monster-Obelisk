#include "iGraphics.h"
#include "creditpage.h"
#include "playpage.h"
/*struct buttonCordinate
{
	int x = 0;
	int y = 0;
}bCordinate[4];*/

int x = 0;
int y = 0;

char homepage[25] = {"Image//homepage.bmp"};
char button[2][20] = { "Image//start.bmp", "Image//credit.bmp" };
char playpage[20] = { "Image//playpage.bmp" };
int gameState = 0;

void iDraw()
{
	iClear();
	iFilledRectangle(0, 0, 1000, 600);
	iSetColor(255, 255, 255);
	if (gameState == 0){
		iShowBMP(0, 0, homepage);
	}
}

void iMouseMove(int mx, int my)
{
	

}

void iPassiveMouseMove(int mx, int my)
{
		
}

void iMouse(int button, int state, int mx, int my)
{printf("%d %d\n", mx, my);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{


	}


	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{

	}
}

// Special Keys:
// GLUT_KEY_F1, GLUT_KEY_F2, GLUT_KEY_F3, GLUT_KEY_F4, GLUT_KEY_F5, GLUT_KEY_F6, GLUT_KEY_F7, GLUT_KEY_F8, GLUT_KEY_F9, GLUT_KEY_F10, GLUT_KEY_F11, GLUT_KEY_F12, 
// GLUT_KEY_LEFT, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_DOWN, GLUT_KEY_PAGE UP, GLUT_KEY_PAGE DOWN, GLUT_KEY_HOME, GLUT_KEY_END, GLUT_KEY_INSERT

void fixedUpdate()
{
	if (isKeyPressed('w') || isSpecialKeyPressed(GLUT_KEY_UP))
	{
		y++;
	}
	if (isKeyPressed('a') || isSpecialKeyPressed(GLUT_KEY_LEFT))
	{
		x--;
	}
	if (isKeyPressed('s') || isSpecialKeyPressed(GLUT_KEY_DOWN))
	{
		y--;
	}
	if (isKeyPressed('d') || isSpecialKeyPressed(GLUT_KEY_RIGHT))
	{
		x++;
	}

	if (isKeyPressed(' ')) {
		// Playing the audio once
		mciSendString("play ggsong from 0", NULL, 0, NULL);
	}
}


int main()
{
	// Opening/Loading the audio files
	//mciSendString("open \"Audios//background.mp3\" alias bgsong", NULL, 0, NULL);
	//mciSendString("open \"Audios//gameover.mp3\" alias ggsong", NULL, 0, NULL);

	// Playing the background audio on repeat
	//mciSendString("play bgsong repeat", NULL, 0, NULL);

	// If the use of an audio is finished, close it to free memory
	// mciSendString("close bgsong", NULL, 0, NULL);
	// mciSendString("close ggsong", NULL, 0, NULL);

	iInitialize(1000, 600, "Project Title");
	iStart();
	return 0;

}
