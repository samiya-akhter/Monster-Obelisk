#define STB_IMAGE_IMPLEMENTATION
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
#include "AdvancedCombatManager.h"
#include "FinalBossManager.h"
#include "characterselection.h"
#include <ctime>
#include "OpenWorld.h"
#include "EishiroConversation.h"
#include "shoppage.h"
#include "nameinputpage.h"
#include "saveselectpage.h"
#include "audio.h"
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
int previousGameState = 0;
int playState = 0;
int creditState = 0;
int settingState = 0;
int helpState = 0;
int wildAreaMode = 0;

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

	// BGM Dispatch
	if (gameState == 0 || (gameState == 1 && playState == 1) || gameState == 1 || gameState == 12 || gameState == 13) {
		AudioSystem::PlayBGM(BGM_MAIN);
	}
	else if (gameState == 10) {
		if (OpenWorldGame::GetInstance().IsTaskShowing()) {
			AudioSystem::PlayBGM(BGM_SHOP);
		} else {
			AudioSystem::PlayBGM(BGM_MAIN);
		}
	}
	else if (gameState == 5 || gameState == 14) {
		AudioSystem::PlayBGM(BGM_SHOP);
	}
	else if (gameState == 8) {
		AudioSystem::PlayBGM(BGM_EXTRA);
	}
	else if (gameState == 6 || gameState == 7 || gameState == 9) {
		bool isDefeat = false;
		bool isVictory = false;
		if (gameState == 6 || gameState == 9) {
			isDefeat = CombatManager::GetInstance().IsDefeat();
			isVictory = CombatManager::GetInstance().IsVictory();
		} else if (gameState == 7) {
			isDefeat = false; // Advanced combat doesn't use a defeat state
			isVictory = AdvancedCombatManager::GetInstance().IsVictory();
		}

		if (isDefeat) AudioSystem::PlayBGM(BGM_LOSE);
		else if (isVictory) AudioSystem::PlayBGM(BGM_WIN);
		else AudioSystem::PlayBGM(BGM_BATTLE);
	}
	else if (gameState == 11) {
		bool isDefeat = FinalBossManager::GetInstance().IsDefeat();
		bool isVictory = FinalBossManager::GetInstance().IsVictory();

		if (isDefeat) AudioSystem::PlayBGM(BGM_LOSE);
		else if (isVictory) AudioSystem::PlayBGM(BGM_WIN);
		else AudioSystem::PlayBGM(BGM_BATTLE);
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
		// Settings gear overlay on spawn map (only when not in story)
		if (playState != 1) {
			iShowImage(918, 518, 42, 42, iLoadImage("Image//setting.bmp", 0, 0, 0));
		}
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
	else if (gameState == 14)
	{
		drawShopMaster();
	}
	else if (gameState == 15)
	{
		drawNameInputPage();
	}
	else if (gameState == 16)
	{
		drawSaveSelectPage();
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

	else if (gameState == 7) { 
		AdvancedCombatManager::GetInstance().Update(deltaTime);
		AdvancedCombatManager::GetInstance().Render();
	}

	else if (gameState == 9)
	{
		CombatManager::GetInstance().UpdateCombat(deltaTime);
		CombatManager::GetInstance().RenderCombat();
	}

	else if (gameState == 10)
	{
		OpenWorldGame::GetInstance().Update(deltaTime);
		OpenWorldGame::GetInstance().Render();
		// Settings gear overlay (top-right corner)
		iShowImage(918, 518, 42, 42, iLoadImage("Image//setting.bmp", 0, 0, 0));
	}
	else if (gameState == 11)
	{
		FinalBossManager::GetInstance().Update(deltaTime);
		FinalBossManager::GetInstance().Render();
	}
	else if (gameState == 12)
	{
		drawCharacterSelection();
	}
	else if (gameState == 13)
	{
		RenderEishiroConversation();
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
			my >= 210 && my <= 265);

		// CREDIT hover
		creditHover = (mx >= 388 && mx <= 578 &&
			my >= 130 && my <= 180);

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

		if (gameState == 4)
			howToPlayPageClick(mx, my);

		if (gameState == 16)
			saveSelectClick(mx, my);

		if (gameState == 3)
			settingPageClick(mx, my);

		//inside map/playpage
		if (gameState == 1) {
			mapClick(mx, my); // Handle clicks on the map
			// Settings gear on spawn map (top-right)
			if (mx >= 918 && mx <= 960 && my >= 518 && my <= 560) {
				previousGameState = gameState;
				gameState = 3;
			}
		}
		
		//inside wildarea1(memorygame)
		if (gameState == 5)
		{
			wildAreaClick(mx, my);
		}

		// Open World task map click + settings gear
		if (gameState == 10)
		{
			OpenWorldGame::GetInstance().HandleMouseClick(mx, my);
			// Settings gear on open world (top-right)
			if (mx >= 918 && mx <= 960 && my >= 518 && my <= 560) {
				previousGameState = gameState;
				gameState = 3;
			}
		}

        // Story Mode Interaction
        if (playState == 1) {
            handleStoryClick(mx, my);
        }

		// Character Selection
		if (gameState == 12) {
			characterSelectionClick(mx, my);
		}
        // Shop selection screen hook
        if (gameState == 14) {
            iMouseShop(button, state, mx, my);
        }
	}

    // Battle Tower / Combat: allow any mouse button to trigger OnCleanClick
    if (state == GLUT_DOWN) {
        if (gameState == 6 || gameState == 9) {
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
    if (gameState == 15) {
        nameInputKeyboardUpdate();
    }

    // Runner Mode Cheat
    if (isKeyPressed('r') || isKeyPressed('R')) {
        gameState = 8;
        RunnerGame::GetInstance().Reset();
    }

    if (gameState == 13) {
        static bool spaceRelConv = true;
        if (isKeyPressed(' ')) {
            if (spaceRelConv) {
                AdvanceConversation();
                spaceRelConv = false;
                if (IsConversationDone()) {
                    gameState = 10;
                }
            }
        } else {
            spaceRelConv = true;
        }
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

	if (gameState == 7) {
		// Player Movement
		if (isKeyPressed('a') || isKeyPressed('A') || isSpecialKeyPressed(GLUT_KEY_LEFT)) {
			AdvancedCombatManager::GetInstance().MovePlayer(-3); // Move Left
		}
		if (isKeyPressed('d') || isKeyPressed('D') || isSpecialKeyPressed(GLUT_KEY_RIGHT)) {
			AdvancedCombatManager::GetInstance().MovePlayer(3);  // Move Right
		}

		// Basic Attack - SPACE (Lightning Blast)
		static bool adv_spaceReleased = true;
		if (isKeyPressed(' ')) {
			if (adv_spaceReleased) {
				AdvancedCombatManager::GetInstance().PlayerAttack(1);
				adv_spaceReleased = false;
			}
		} else {
			adv_spaceReleased = true;
		}

		// Thunder Crash - key 'F' (Wave 3+)
		static bool fKeyReleased = true;
		if (isKeyPressed('f') || isKeyPressed('F')) {
			if (fKeyReleased) {
				AdvancedCombatManager::GetInstance().PlayerAttack(2);
				fKeyReleased = false;
			}
		} else {
			fKeyReleased = true;
		}
	}

	if (gameState == 11) {
		// Player Movement (Faster for Sprint)
		if (isKeyPressed('a') || isKeyPressed('A') || isSpecialKeyPressed(GLUT_KEY_LEFT)) {
			FinalBossManager::GetInstance().MovePlayer(-10); // Move Left (Faster)
		}
		if (isKeyPressed('d') || isKeyPressed('D') || isSpecialKeyPressed(GLUT_KEY_RIGHT)) {
			FinalBossManager::GetInstance().MovePlayer(10);  // Move Right (Faster)
		}

		// Basic Attack - W
		static bool fb_wKeyReleased = true;
		if (isKeyPressed('w') || isKeyPressed('W')) {
			if (fb_wKeyReleased) {
				FinalBossManager::GetInstance().PlayerAttack(1);
				fb_wKeyReleased = false;
			}
		} else {
			fb_wKeyReleased = true;
		}

		// Jump - SPACE
		if (isKeyPressed(' ')) {
			FinalBossManager::GetInstance().JumpPlayer();
		}

		// Thunder Crash - F
		static bool fb_fKeyReleased = true;
		if (isKeyPressed('f') || isKeyPressed('F')) {
			if (fb_fKeyReleased) {
				FinalBossManager::GetInstance().PlayerAttack(2);
				fb_fKeyReleased = false;
			}
		} else {
			fb_fKeyReleased = true;
		}

		// Global Restart/Exit for Boss Room via space/ESC handled by keyboard inputs natively or here
        // (Wait, space is used for attack and restart. We need to distinguish it.)
        // But let's just let iKeyboard or fixedUpdate do it:
        FinalBossManager::GetInstance().HandleInput(isKeyPressed(' ') ? ' ' : (isKeyPressed(27) ? 27 : 0));
        
        static bool bossEsc = true;
        if (isKeyPressed(27)) {
            if (bossEsc) {
               gameState = 10;
               bossEsc = false;
            }
        } else { bossEsc = true; }
	}



	// Global movements
	if (isKeyPressed('w') || isSpecialKeyPressed(GLUT_KEY_UP))
	{
		if (gameState == 6 || gameState == 9) {
			CombatManager::GetInstance().MovePlayer(0, 3);
		} else if (gameState == 7) {
			// W = attack in Tower 2
			// handled via debounce below
		} else {
			y++;
		}
	}
	if (isKeyPressed('a') || isSpecialKeyPressed(GLUT_KEY_LEFT))
	{
		if (gameState == 6 || gameState == 9) {
			CombatManager::GetInstance().MovePlayer(-3, 0);
		} else {
			x--;
		}
	}
	if (isKeyPressed('s') || isSpecialKeyPressed(GLUT_KEY_DOWN))
	{
		if (gameState == 6 || gameState == 9) {
			CombatManager::GetInstance().MovePlayer(0, -3);
		} else {
			y--;
		}
	}
	if (isKeyPressed('d') || isSpecialKeyPressed(GLUT_KEY_RIGHT))
	{
		if (gameState == 6 || gameState == 9) {
			CombatManager::GetInstance().MovePlayer(3, 0);
		} else {
			x++;
		}
	}

	static bool wAttackReleased = true;
    if (isKeyPressed('w') || isKeyPressed('W')) {
        if (wAttackReleased) {
            if (gameState == 6 || gameState == 9) {
                CombatManager::GetInstance().PlayerAttack(1); // Type 1: Lightning Blast
            }
            if (gameState == 7) {
                AdvancedCombatManager::GetInstance().PlayerAttack(1);
            }
            wAttackReleased = false;
        }
    } else {
        wAttackReleased = true;
    }

	static bool spaceReleased = true; // Debounce flag
    if (isKeyPressed(' ')) {
        if (spaceReleased) {
            if (playState == 1) { // Story Mode
                nextStorySlide();
            }
            spaceReleased = false; // Block until released
        }
	} else {
        spaceReleased = true; // Key released, reset flag
    }

	static bool hReleased = true;
	if (isKeyPressed('h') || isKeyPressed('H')) {
		if (hReleased) {
			CombatManager& cm = CombatManager::GetInstance();
			if (cm.healPotionCount > 0 && (gameState == 6 || gameState == 7 || gameState == 9 || gameState == 11)) {
				cm.healPotionCount--;
                if (gameState == 6 || gameState == 9) {
				    cm.HealPlayer(0.5f);
                } else if (gameState == 7) {
                    AdvancedCombatManager::GetInstance().HealPlayer(0.5f);
                } else if (gameState == 11) {
                    FinalBossManager::GetInstance().HealPlayer(0.5f);
                }
			}
			hReleased = false;
		}
	} else { hReleased = true; }

	static bool xReleased = true;
	if (isKeyPressed('x') || isKeyPressed('X')) {
		if (xReleased) {
			CombatManager& cm = CombatManager::GetInstance();
			if (cm.damagePotionCount > 0 && !cm.damagePotionUsed && (gameState == 6 || gameState == 7 || gameState == 9 || gameState == 11)) {
				cm.damagePotionCount--;
				cm.damagePotionUsed = true;
			}
			xReleased = false;
		}
	} else { xReleased = true; }
	
	if (isKeyPressed('f') || isKeyPressed('F')) {
		if (gameState == 6 || gameState == 9) {
			CombatManager::GetInstance().PlayerAttack(2); // Type 2: Thunder Crash (Wave 3+)
		}
		if (gameState == 7) {
			AdvancedCombatManager::GetInstance().PlayerAttack(2); // Thunder Crash in Tower 2
		}
	}

    // Global Exit Handling with ESC
    static bool escReleased = true;
    if (isKeyPressed(27)) { // 27 is ESC key
            if (escReleased) {
            if (gameState == 5) {
                gameState = 10; // Return to Open World Map
            } else if (gameState == 8) {
                gameState = 5; // Return to Selection
                RunnerGame::GetInstance().Reset();
            } else if (gameState == 6 || gameState == 7 || gameState == 9) {
                bool canReturn = false;
                if (gameState == 6 || gameState == 9) {
                    canReturn = CombatManager::GetInstance().IsVictory();
                } else if (gameState == 7) {
                    canReturn = AdvancedCombatManager::GetInstance().IsVictory();
                }
                if (canReturn) {
                    // Mark tower as cleared
                    CombatManager& cm = CombatManager::GetInstance();
                    if (gameState == 6)  cm.tower1Cleared = true;
                    if (gameState == 7)  cm.tower2Cleared = true;
                    if (gameState == 9)  cm.tower3Cleared = true;
                    if (cm.tower1Cleared && cm.tower2Cleared && cm.tower3Cleared)
                        cm.allTowersCleared = true;

                    gameState = 10; // Return to open world map
                    OpenWorldGame::GetInstance().SetTriggerCooldown(1.5);
                }
            } else if (gameState == 11) {
                if (FinalBossManager::GetInstance().IsVictory()) {
                    SaveManager::SaveGame(SaveManager::currentPlayerName);
                    gameState = 0; // Return to main menu
                }
            } else if (gameState == 10) {
                gameState = 0;  // Return to main menu
            }
            escReleased = false;
        }
    } else {
        escReleased = true;
    }

    // Global Enter Handling for Tower Triggers
    static bool enterReleased = true;
    if (isKeyPressed('\r') || isKeyPressed(13)) {
        if (enterReleased && gameState == 10) {
            CombatManager& cm = CombatManager::GetInstance();

            // Endgame: houses trigger (all towers cleared, quests not done yet)
            if (cm.allTowersCleared && !(cm.endgameRunnerDone && cm.endgameMemoryDone)
                && OpenWorldGame::GetInstance().CheckForHousesTrigger()) {
                
                cm.lastTowerPlayed = 4; // Specify Endgame Quest

                wildAreaMode = 0; // Show selection screen
                gameState = 5;   // Enter wild area in endgame mode
            }
            // Wild Area (Top Right)
            else if (OpenWorldGame::GetInstance().CheckForWildAreaTrigger()) {
                wildAreaMode = 0; // Selection screen
                gameState = 5;
            }
            // Tower 3 (Yellow)
            else if (OpenWorldGame::GetInstance().CheckForBattleTower3Trigger()) {
                if (cm.lives > 0) {
                    cm.InitTower3();
                    gameState = 9; // Tower 3
                }
            }
            // Tower 2 (Green)
            else if (OpenWorldGame::GetInstance().CheckForBattleTower2Trigger()) {
                shopMenuState = 0;
                gameState = 14; // Go to selection / shop
            }
            // Tower 1 (Red Zone)
            else if (OpenWorldGame::GetInstance().CheckForBattleTower1Trigger()) {
                if (cm.lives > 0) {
                    cm.InitCombat();
                    gameState = 6; // Tower 1
                }
            }
            enterReleased = false;
        }
    } else {
        enterReleased = true;
    }
}
 

void openWorldAnimTimer() {
    if (gameState == 10) {
        OpenWorldGame::GetInstance().UpdateAnimation();
    }
}

int main()
{
	// Opening/Loading the audio files
	AudioSystem::Init();

	iSetTimer(30, updateStory);
	iSetTimer(800, updatePlayPage);
	iSetTimer(100, openWorldAnimTimer);
	srand(time(NULL));
	shuffleCards(); // Randomize cards at start
	iSetTimer(20, checkMemoryMatch);

	iInitialize(1000, 600, "Monstrum Obelisk"); // Creates OpenGL Context
	
	// Initialize Combat System AFTER OpenGL context is created
	CombatManager::GetInstance().InitCombat();
    RunnerGame::GetInstance().Init();
	AdvancedCombatManager::GetInstance().Init();
	FinalBossManager::GetInstance().Init();
	OpenWorldGame::GetInstance().Init();

	iStart();
	return 0;
}