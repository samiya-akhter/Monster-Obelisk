#include "iGraphics.h"
#include "CombatManager.h" 
#include "menu.h"
#include "creditpage.h"
#include "playpage.h"
#include "setting.h"
#include "howtoplaypage.h"
#include "wildarea.h"
#include "battletower1.h"
#include "RunnerGame.h"
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

void playClick(int mx, int my);
void creditClick(int mx, int my);
void settingClick(int mx, int my);
void howToPlayClick(int mx, int my);

int playbutton = 0;
int creditbutton = 0;

int x = 0;
int y = 0;

char homepage[25] = {"Image//homepage.png"};
char button[2][20] = { "Image//play.bmp", "Image//credit.bmp" };
//char playpage[20] = { "Image//playpage.bmp" };
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

gameState = 7 >> story sequence
*/


// Time tracking for DeltaTime
clock_t lastTime = 0;

void iDraw()
{
	iClear();
	iFilledRectangle(0, 0, 1000, 600);
	iSetColor(255, 255, 255);

    // DeltaTime Calculation
    clock_t currentTime = clock();
    if (lastTime == 0) lastTime = currentTime;
    float deltaTime = (float)(currentTime - lastTime) / CLOCKS_PER_SEC;
    lastTime = currentTime;

    // Update Combat System always (or only when needed)
    // Safer to only update inside current state or keep it running if it has internal state checks
    if (gameState == 6) {
        CombatManager::GetInstance().UpdateCombat(deltaTime);
    }

	if (gameState == 0)
	{
		drawMenu();
	}
/*	else if (playState == 1)
	{
		drawStory();
	}*/
	else if (gameState == 1 || playState == 1 )
//	else if (gameState == 1)
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
		// drawBattleTower1(); // Replaced with Combat System
        CombatManager::GetInstance().RenderCombat();
	}

    else if (gameState == 8) 
    {
        RunnerGame::GetInstance().Update(deltaTime);
        RunnerGame::GetInstance().Render();
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
	 printf("%d %d\n", mx, my); // Debug printing

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) 
    {
		
		

		// Other pages
		if (gameState == 0)
			playClick(mx, my);
		if (gameState == 0) // Should check if credit button clicked
			creditClick(mx, my);

		if (gameState == 2)// Should check if credit button clicked
			backClick(mx, my);

		if (gameState == 0) // ...
			settingClick(mx, my);
		if (gameState == 0)
			howToPlayClick(mx, my);

		if (gameState == 3)
			settingBackClick(mx, my);
        // Setting Page Interactions
       /* if (gameState == 3) {

			settingClick(mx, my);
        }*/


		//shows the story
/*		if (playState == 1) {
			storyClick(mx, my, playState,gameState); 
			return;
		}*/

		//inside map/playpage
		if (gameState == 1) {
			mapClick(mx, my); // Handle clicks on the map
		}
		
		//inside wildarea1(memorygame)
		if (gameState == 5)
		{
			wildAreaClick(mx, my);
		}

        // Story Mode Interaction
        if (playState == 1) {
            handleStoryClick(mx, my);
        }

        // Battle Tower / Combat
        if (gameState == 6) {
            CombatManager::GetInstance().OnCleanClick(mx, my);
        }
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

// GLUT_KEY_LEFT, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_DOWN, GLUT_KEY_PAGE UP, GLUT_KEY_PAGE DOWN, GLUT_KEY_HOME, GLUT_KEY_END, GLUT_KEY_INSERT

void fixedUpdate()
{
    // Runner Mode Cheat
    if (isKeyPressed('r') || isKeyPressed('R')) {
        gameState = 8;
        RunnerGame::GetInstance().Reset();
    }

    if (gameState == 8) {
        if (isKeyPressed(' ')) {
             RunnerGame::GetInstance().HandleInput(' ');
        }
        
        static bool cKeysReleased = true;
        if (isKeyPressed('c') || isKeyPressed('C')) {
            if (cKeysReleased) {
                RunnerGame::GetInstance().HandleInput('c');
                cKeysReleased = false;
            }
        } else {
             cKeysReleased = true;
        }

        if (isKeyPressed(27)) { // ESC
            gameState = 5; // Back to Wild Area Selection
            wildAreaMode = 0; // Ensure selection mode
        }
        return; // Stop processing other fixedUpdates
    }

	// Global movements
	if (isKeyPressed('w') || isSpecialKeyPressed(GLUT_KEY_UP))
	{
		if (gameState == 6) {
			CombatManager::GetInstance().MovePlayer(0, 5);
		} else {
			y++;
		}
	}
	if (isKeyPressed('a') || isSpecialKeyPressed(GLUT_KEY_LEFT))
	{
		if (gameState == 6) {
			CombatManager::GetInstance().MovePlayer(-5, 0);
		} else {
			x--;
		}
	}
	if (isKeyPressed('s') || isSpecialKeyPressed(GLUT_KEY_DOWN))
	{
		if (gameState == 6) {
			CombatManager::GetInstance().MovePlayer(0, -5);
		} else {
			y--;
		}
	}
	if (isKeyPressed('d') || isSpecialKeyPressed(GLUT_KEY_RIGHT))
	{
		if (gameState == 6) {
			CombatManager::GetInstance().MovePlayer(5, 0);
		} else {
			x++;
		}
	}

	static bool spaceReleased = true; // Debounce flag
    if (isKeyPressed(' ')) {
        if (spaceReleased) {
            if (gameState == 6) {
                CombatManager::GetInstance().PlayerAttack(1); // Type 1: Lightning Blast
            }
            if (playState == 1) { // Story Mode
                nextStorySlide();
            }
            spaceReleased = false; // Block until released
        }
		// Playing the audio once
		//mciSendString("play ggsong from 0", NULL, 0, NULL);
	} else {
        spaceReleased = true; // Key released, reset flag
    }
	
	if (isKeyPressed('f')) {
		if (gameState == 6) {
			CombatManager::GetInstance().PlayerAttack(2); // Type 2: Thunder Crash (Wave 3+)
		}
	}

    // Exit Combat with ESC
    if (isKeyPressed(27)) { // 27 is ESC key
        if (gameState == 6) {
             gameState = 1; // Return to map/play page
        }
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
   // mciSendString("open \"Audios//main.mp3\" alias mainMusic", NULL, 0, NULL);
    //mciSendString("play mainMusic repeat", NULL, 0, NULL);

	iSetTimer(30, updateStory);
	iSetTimer(800, updatePlayPage);
	srand(time(NULL));
	shuffleCards(); // Randomize cards at start
	iSetTimer(20, checkMemoryMatch);

	iInitialize(1000, 600, "Monstrum Obelisk"); // Creates OpenGL Context
	
	// Initialize Combat System AFTER OpenGL context is created
	CombatManager::GetInstance().InitCombat();
    RunnerGame::GetInstance().Init();

	iStart();
	return 0;
}