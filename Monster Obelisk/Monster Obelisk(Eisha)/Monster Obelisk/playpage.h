#ifndef PLAYPAGE_H
#define PLAYPAGE_H
#include "AdvancedCombatManager.h"
#include "OpenWorld.h"
#include "CombatManager.h"
extern int playState;
extern int gameState;

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

int playFrame = 0;

void drawPlayPage()
{
	iShowImage(0, 0, 1000, 600, iLoadImage(map[playFrame]));

	// Show "No Lives" warning if player cannot fight
	if (CombatManager::GetInstance().lives <= 0) {
		iSetColor(255, 0, 0);
		iText(320, 580, "NO LIVES! Win Memory Game in Wild Area!", (void*)0x0005);
	}
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
