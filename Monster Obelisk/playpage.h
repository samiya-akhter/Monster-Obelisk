#ifndef PLAYPAGE_H
#define PLAYPAGE_H
#include "AdvancedCombatManager.h"
#include "OpenWorld.h"
#include "CombatManager.h"
extern int playState;
extern int gameState;

char storySlides[8][30] = {
	"image//story1.bmp",
	"image//story2.bmp",
	"image//story3.bmp",
	"image//story4.bmp",
	"image//story5.bmp",
	"image//story6.bmp",
	"image//story7.bmp",
	"image//story8.bmp"
};


char map[8][25] = {
	"image//playpage1.png",
	"image//playpage2.png",
	"image//playpage3.png",
	"image//playpage4.png",
	"image//playpage5.png",
	"image//playpage6.png",
	"image//playpage7.png",
	"image//playpage8.png"
};

int storyFrame = 0;

int playFrame = 0;

// Button coordinates
int skipX = 850, skipY = 550, skipW = 100, skipH = 40;

void drawPlayPage()
{
	if (playState == 1) {
        iShowImage(0, 0, 1000, 600, iLoadImage(storySlides[storyFrame]));
        
        // Skip Button
        iSetColor(200, 50, 50);
        iFilledRectangle(skipX, skipY, skipW, skipH);
        iSetColor(255, 255, 255);
        iText(skipX + 30, skipY + 15, "SKIP", (void*)0x0008);
        
        // Instruction
        iText(400, 570, "Press SPACE to Continue", (void*)0x0008);
    }
	else {
		iShowImage(0, 0, 1000, 600, iLoadImage(map[playFrame]));
        
        // Show "No Lives" warning if player cannot fight
        if (CombatManager::GetInstance().lives <= 0) {
            iSetColor(255, 0, 0);
            iText(320, 580, "NO LIVES! Win Memory Game in Wild Area!", (void*)0x0005);
        }
	}
}

void nextStorySlide() {
    storyFrame++;
    if (storyFrame >= 8) {
        playState = 2; // End story state
        OpenWorldGame::GetInstance().Init();
        gameState = 10; // Transition to Open World
        storyFrame = 0; // Reset for future use
    }
}

void skipStory() {
    playState = 2;
    OpenWorldGame::GetInstance().Init();
    gameState = 10;
    storyFrame = 0;
}

void handleStoryClick(int mx, int my) {
    if (mx >= skipX && mx <= skipX + skipW && my >= skipY && my <= skipY + skipH) {
        skipStory();
    }
}

void updateStory()
{
	// Auto-increment removed for manual control
}

void updatePlayPage()
{
	if (playState == 2){
		playFrame++;
		if (playFrame >= 8)
			playFrame = 0;

	}
}

void mapClick(int mx, int my) {
	// Coordinates for Right Tower / Open World
	if (mx >= 790 && mx <= 850 && my >= 220 && my <= 360) {
		OpenWorldGame::GetInstance().Init();
		gameState = 10;
	}
}
#endif
