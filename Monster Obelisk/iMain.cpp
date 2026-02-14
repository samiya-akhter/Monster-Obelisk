#include "iGraphics.h"
#include "menu.h"
#include "creditpage.h"
#include "playpage.h"
#include "setting.h"
#include "howtoplaypage.h"
#include "wildarea.h"
#include "battletower1.h"
#include <ctime>

void drawPlayPage();
void drawCreditPage();
void drawSettingPage();
void drawHowToPlayPage();
void drawWildArea();
void drawBattleTower1();


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

/*
gameState = 0 >> main menu
gameState = 1 >> playpage/map
gameState = 2 >> creditpage
gameState = 3 >> settings
gameState = 4 >> how to play
gameState = 5 >> wild area
gameState = 6 >> battle tower 1

*/



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
		drawSettingPage();
	}

	else if (gameState == 4)
	{
		drawHowToPlayPage();
	}

	else if (gameState == 5)
	{
		drawWildArea();
	}

	else if (gameState == 6)
	{
		drawBattleTower1();
	}
}


void checkMemoryMatch() {
	if (lockBoard && firstIdx != -1 && secondIdx != -1) {
		static int timerCount = 0;
		timerCount++;

		if (timerCount > 20) { // Provides a delay so you can see the cards
			if (cards[firstIdx] == cards[secondIdx]) {
				cardState[firstIdx] = 2; // Match found
				cardState[secondIdx] = 2;
				matchesFound++;
			}
			else {
				cardState[firstIdx] = 0; // Flip back
				cardState[secondIdx] = 0;
			}
			// Reset for next turn
			firstIdx = -1;
			secondIdx = -1;
			lockBoard = 0;
			timerCount = 0;
		}
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
	if (gameState == 0)  // MENU
	{
		// PLAY hover
		playHover = (mx >= 398 && mx <= 572 &&
			my >= 160 && my <= 215);

		// CREDIT hover
		creditHover = (mx >= 388 && mx <= 578 &&
			my >= 80 && my <= 130);

		// SETTINGS hover
		settingHover = (mx >= 918 && mx <= 960 &&
			my >= 518 && my <= 560);

		// HOW TO PLAY hover
		helpHover = (mx >= 38 && mx <= 110 &&
			my >= 36 && my <= 90);
	}

	// Credit page hover (back button)
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

		//inside map/playpage
		if (gameState == 1) {
			mapClick(mx, my); // Handle clicks on the map
		}
		
		//inside wildarea1(memorygame)
		if (gameState == 5)
		{
			wildAreaClick(mx, my);
		}

	


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
	srand(time(NULL));
	shuffleCards(); // Randomize cards at start
	iSetTimer(20, checkMemoryMatch);

	iInitialize(1000, 600, "Monstrum Obelisk");
	iStart();
	return 0;
}