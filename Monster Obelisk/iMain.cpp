#include "iGraphics.h"
#include "menu.h"
#include "creditpage.h"
#include "playpage.h"
#include "setting.h"
#include "howtoplaypage.h"


/*struct buttonCordinate
{
	int x = 0;
	int y = 0;
}bCordinate[4];*/
void playPage();
void creditPage();
void settingPage();

void playClick();
void creditClick();

int playbutton = 0;
int creditbutton = 0;

int x = 0;
int y = 0;

char homepage[25] = {"Image//homepage.bmp"};
char button[2][20] = { "Image//play.bmp", "Image//credit.bmp" };
char playpage[20] = { "Image//playpage.bmp" };
int gameState = 0;
int playState = 0;
int creditState = 0;
int settingState = 0;
int helpState = 0;


void iDraw()
{
	iClear();
	iFilledRectangle(0, 0, 1000, 600);
	iSetColor(255, 255, 255);
	if (gameState == 0)
	{
		drawMenu();
	}
	else if (gameState == 1)
	{
		drawPlayPage();
	}

	else if (gameState == 2)
	{
		drawCreditPage();
	}


	else if (gameState == 3)
	{
		drawSettingsPage();
	}

	else if (gameState == 4)
	{
		drawHowToPlayPage();
	}

}

void iMouseMove(int mx, int my)
{
	/*if (button == GLUT_LEFT_BUTTON)
	{
		if (gameState == 2)
			creditPassiveMouseMove(mx, my);
	}*/

}

void iPassiveMouseMove(int mx, int my)
{
	if (gameState == 0)  // Menu
	{
		// Play button hover
		if (mx >= 398 && mx <= 572 && my >= 160 && my <= 215)
			playHover = 1;
		else
			playHover = 0;

		// Credit button hover
		if (mx >= 388 && mx <= 578 && my >= 80 && my <= 130)
			creditHover = 1;
		else
			creditHover = 0;
	}

	// Credit Page hover
	if (gameState == 2)
		creditPassiveMouseMove(mx, my);
}

void iMouse(int button, int state, int mx, int my)
{
	printf("%d %d\n", mx, my);

	
		
		if (gameState == 2)  // Credit Page
			creditMouse(button, state, mx, my);  // pass button & state here

		// Other pages
		if (gameState == 0)
			playClick(mx, my);
		if (gameState == 0)
			creditClick(mx, my);
		if (gameState == 0)
			settingClick(mx, my);
		if (gameState == 0)
			howToPlayClick(mx, my);
		

	


	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{

	}


	
}

void iSpecialKeyboard(unsigned char key){
	
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
		//mciSendString("play ggsong from 0", NULL, 0, NULL);
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
	iSetTimer(800, updatePlayPage);
	iInitialize(1000, 600, "Monstrum Obelisk");
	iStart();
	return 0;
}