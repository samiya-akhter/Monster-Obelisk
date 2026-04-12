#ifndef SAVESELECTPAGE_H
#define SAVESELECTPAGE_H

#include "iGraphics.h"
#include "SaveManager.h"

extern int gameState; // 16
extern int playState;

static unsigned int saveSelectBgImg = 0;

void drawSaveSelectPage() {
    if (saveSelectBgImg == 0) {
        saveSelectBgImg = iLoadImage("Image//save_select_bg.png");
    }
    
    iShowImage(0, 0, 1000, 600, saveSelectBgImg);

    // Render "NEW GAME" Box (Left Side)
    iSetColor(50, 50, 50);
    iFilledRectangle(150, 250, 300, 100);
    iSetColor(255, 215, 0); // Gold Border
    iRectangle(150, 250, 300, 100);
    iSetColor(255, 255, 255);
    iText(150 + 300 / 2 - (8 * 9) / 2, 250 + 100 / 2 - 15 / 2, "NEW GAME", (void*)0x0002);
    
    // Render "CONTINUE" Box (Right Side)
    iSetColor(50, 50, 50);
    iFilledRectangle(550, 250, 300, 100);
    iSetColor(255, 215, 0); // Gold Border
    iRectangle(550, 250, 300, 100);
    iSetColor(255, 255, 255);
    
    std::string s = "CONTINUE: " + SaveManager::GetSavedPlayerName();
    int sWidth = s.length() * 9;
    iText(550 + 300 / 2 - sWidth / 2, 250 + 100 / 2 - 15 / 2, s.c_str(), (void*)0x0002);

    // BACK BUTTON
	iSetColor(200, 0, 0);
	iFilledRectangle(20, 540, 80, 40);
	iSetColor(255, 255, 255);
	iText(20 + 80 / 2 - (4 * 9) / 2, 540 + 40 / 2 - 15 / 2, "BACK", (void*)0x0002);
}

void saveSelectClick(int mx, int my) {
    if (mx >= 150 && mx <= 450 && my >= 250 && my <= 350) {
        // NEW GAME -> Name input
        gameState = 15; 
    }
    else if (mx >= 550 && mx <= 850 && my >= 250 && my <= 350) {
        // CONTINUE -> Load game and jump to game map (gameState=10 or playState=1 sequence)
        SaveManager::LoadGame();
        gameState = 10; // Open World Map if continued
    }
    // Back button
	if (mx >= 20 && mx <= 100 && my >= 540 && my <= 580) {
		gameState = 0; // Return to Main Menu
	}
}

#endif
