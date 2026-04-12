#ifndef WILDAREAPAGE_H
#define WILDAREAPAGE_H

#include <cstdlib>
#include <stdio.h>
#include "CombatManager.h" // Required for RestoreLives()
#include "RunnerGame.h"    // Required for RunnerGame::GetInstance()
#include "AdvancedCombatManager.h"
#include "iGraphics.h"

extern int gameState;

// Game variables
static int cards[12] = { 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6 };
static int cardState[12] = { 0 }; // 0: Hidden, 1: Flipped, 2: Matched
static int firstIdx = -1, secondIdx = -1;
static int lockBoard = 0; // Prevents clicking during animation
static int matchesFound = 0;
static int movesLeft = 10;
const int MAX_MOVES = 12;

extern int wildAreaMode;

static void shuffleCards() {
	// Reset game state
	matchesFound = 0;
	movesLeft = MAX_MOVES;
	firstIdx = -1;
	secondIdx = -1;
	lockBoard = 0;
	for (int k = 0; k<12; k++) cardState[k] = 0;

	for (int i = 11; i > 0; i--) {
		int j = rand() % (i + 1);

		int temp = cards[i];
		cards[i] = cards[j];
		cards[j] = temp;
	}
}

static void restartWildArea() {
	shuffleCards();
}

static void drawWildAreaSelection() {
	// Background
	iShowImage(0, 0, 1000, 600, iLoadImage("Image//Side quest bg.png"));

	// Title text with game-style drop shadow
	iSetColor(0, 0, 0); // Shadow
	iText(412, 498, "CHOOSE YOUR ACTIVITY", (void*)0x0002);
	iSetColor(255, 215, 0); // Gold theme color
	iText(410, 500, "CHOOSE YOUR ACTIVITY", (void*)0x0002);

	// Button 1: Memory Game
	// Drop shadow
	iSetColor(32, 92, 82); // Darker Teal
	iFilledCircle(224, 336, 40);
	iFilledCircle(424, 336, 40);
	iFilledRectangle(224, 296, 200, 80);
	// Base button body
	iSetColor(52, 112, 102); // Dark Muted Teal
	iFilledCircle(220, 340, 40);
	iFilledCircle(420, 340, 40);
	iFilledRectangle(220, 300, 200, 80);
	// Top Highlight (Light Reflection)
	iSetColor(72, 132, 122); // Teal Highlight
	iFilledCircle(220, 360, 20);
	iFilledCircle(420, 360, 20);
	iFilledRectangle(220, 340, 200, 40);

	// Text with Chunky Bubble Outline
	iSetColor(0, 0, 0);
	iText(272, 342, "MEMORY GAME", (void*)0x0002);
	iText(264, 313, "Restore Lives", (void*)0x0002);
	iSetColor(255, 255, 255);
	iText(270, 344, "MEMORY GAME", (void*)0x0002);
	iText(262, 315, "Restore Lives", (void*)0x0002);

	// Button 2: Side Quest (Runner)
	// Drop shadow
	iSetColor(96, 115, 76); // Darker Sage
	iFilledCircle(584, 336, 40);
	iFilledCircle(784, 336, 40);
	iFilledRectangle(584, 296, 200, 80);
	// Base button body
	iSetColor(116, 135, 96); // Dark Sage Green
	iFilledCircle(580, 340, 40);
	iFilledCircle(780, 340, 40);
	iFilledRectangle(580, 300, 200, 80);
	// Top Highlight
	iSetColor(136, 155, 116); // Sage Highlight
	iFilledCircle(580, 360, 20);
	iFilledCircle(780, 360, 20);
	iFilledRectangle(580, 340, 200, 40);

	// Text with Chunky Bubble Outline
	iSetColor(0, 0, 0);
	iText(637, 342, "SIDE QUEST", (void*)0x0002);
	iText(610, 313, "Collect Crystals", (void*)0x0002);
	iSetColor(255, 255, 255);
	iText(635, 344, "SIDE QUEST", (void*)0x0002);
	iText(608, 315, "Collect Crystals", (void*)0x0002);

	// Back Button
	// Drop shadow
	iSetColor(50, 70, 100); // Darker Blue
	iFilledCircle(43, 557, 20);
	iFilledCircle(123, 557, 20);
	iFilledRectangle(43, 537, 80, 40);
	// Base body
	iSetColor(70, 90, 120); // Dark Pastel Blue
	iFilledCircle(40, 560, 20);
	iFilledCircle(120, 560, 20);
	iFilledRectangle(40, 540, 80, 40);
	// Highlight
	iSetColor(90, 110, 140);
	iFilledCircle(40, 570, 10);
	iFilledCircle(120, 570, 10);
	iFilledRectangle(40, 560, 80, 20);

	// Text with Chunky Bubble Outline
	iSetColor(0, 0, 0);
	iText(64, 551, "BACK", (void*)0x0002);
	iSetColor(255, 255, 255);
	iText(62, 553, "BACK", (void*)0x0002);
}

static void drawMemoryGame() {
	iShowBMP(0, 0, "Image//wildarea1.bmp");

	// Game Title
	iSetColor(255, 215, 0); // Gold
	iText(379, 580, "Wild Area: Memory Challenge", (void*)0x0002);

	// Moves Left
	char movesBuf[32];
	sprintf_s(movesBuf, 32, "Moves Left: %d", movesLeft);
	if (movesLeft <= 3) iSetColor(255, 50, 50); // Warning color
	else iSetColor(255, 255, 255);
	iText(437, 550, movesBuf, (void*)0x0002);

	// Pairs Found
	char pairsBuf[32];
	sprintf_s(pairsBuf, 32, "Pairs Found: %d/6", matchesFound);
	iSetColor(0, 255, 255); // Cyan
	iText(428, 520, pairsBuf, (void*)0x0002);

	for (int i = 0; i < 12; i++) {
		int xPos = 180 + (i % 4) * 180;
		int yPos = 400 - (i / 4) * 150;

		if (cardState[i] == 0) {
			iShowBMP2(xPos, yPos, "Image//card_back.bmp", 0);
		}
		else if (cardState[i] == 1) {
			char cardImg[30];
			sprintf_s(cardImg, sizeof(cardImg), "Image//poke%d.bmp", cards[i]);
			iShowBMP2(xPos, yPos, cardImg, 0);
		}
		else if (cardState[i] == 2){
			continue;
		}
	}

	// Back Button (Returns to Selection now)
	// Drop shadow
	iSetColor(50, 70, 100);
	iFilledCircle(43, 557, 20);
	iFilledCircle(123, 557, 20);
	iFilledRectangle(43, 537, 80, 40);
	// Base body
	iSetColor(70, 90, 120);
	iFilledCircle(40, 560, 20);
	iFilledCircle(120, 560, 20);
	iFilledRectangle(40, 540, 80, 40);
	// Highlight
	iSetColor(90, 110, 140);
	iFilledCircle(40, 570, 10);
	iFilledCircle(120, 570, 10);
	iFilledRectangle(40, 560, 80, 20);
	// Text
	iSetColor(0, 0, 0);
	iText(64, 551, "BACK", (void*)0x0002);
	iSetColor(255, 255, 255);
	iText(62, 553, "BACK", (void*)0x0002);

	// Restart Button
	// Drop shadow
	iSetColor(50, 70, 100);
	iFilledCircle(883, 517, 20);
	iFilledCircle(963, 517, 20);
	iFilledRectangle(883, 497, 80, 40);
	// Base body
	iSetColor(70, 90, 120);
	iFilledCircle(880, 520, 20);
	iFilledCircle(960, 520, 20);
	iFilledRectangle(880, 500, 80, 40);
	// Highlight
	iSetColor(90, 110, 140);
	iFilledCircle(880, 530, 10);
	iFilledCircle(960, 530, 10);
	iFilledRectangle(880, 520, 80, 20);
	// Text
	iSetColor(0, 0, 0);
	iText(891, 511, "RESTART", (void*)0x0002);
	iSetColor(255, 255, 255);
	iText(889, 513, "RESTART", (void*)0x0002);

	// Win State
	if (matchesFound == 6) {
		static unsigned int fullEnergyTex = 0;
		if (fullEnergyTex == 0) {
			fullEnergyTex = iLoadImage("Image//full_energy.bmp", 0, 0, 0);
		}
		iShowImage(100, 60, 800, 480, fullEnergyTex);

		iSetColor(0, 255, 0);
		if (CombatManager::GetInstance().allTowersCleared && CombatManager::GetInstance().endgameRunnerDone) {
			iText(361, 120, "Quest Complete! Journey awaits!", (void*)0x0002);
			iSetColor(255, 215, 0); // Gold
			iText(307, 80, "Press BACK to return to the main world map!", (void*)0x0002);
		}
		else {
			iSetColor(255, 215, 0); // Gold
			iText(329, 80, "Press BACK to resume battle with Vivi!", (void*)0x0002);
		}
	}
	// Loss State
	else if (movesLeft <= 0 && lockBoard == 0) {
		static unsigned int failedCardTex = 0;
		if (failedCardTex == 0) {
			failedCardTex = iLoadImage("Image//failedCard.bmp", 0, 0, 0);
		}
		// Scaled exactly like the win screen (800x480) and centered
		iShowImage(100, 60, 800, 480, failedCardTex);
	}

	// Show current lives
	char livesBuf[32];
	sprintf_s(livesBuf, 32, "Lives: %d", CombatManager::GetInstance().lives);
	iSetColor(255, 215, 0); // Gold
	iText(880, 560, livesBuf, (void*)0x0002);
}

static void drawWildArea()
{
	// If RunnerGame signals we should jump straight to memory game
	if (CombatManager::GetInstance().pendingMemoryGame) {
		CombatManager::GetInstance().pendingMemoryGame = false;
		shuffleCards();
		wildAreaMode = 1;
	}

	if (wildAreaMode == 0) {
		drawWildAreaSelection();
	}
	else {
		drawMemoryGame();
	}
}

static void wildAreaClick(int mx, int my) {
	if (wildAreaMode == 0) {
		// Selection Screen Logic

		// Memory Game Button
		if (mx >= 180 && mx <= 460 && my >= 300 && my <= 380) {
			wildAreaMode = 1; // Start Memory Game
			restartWildArea(); // Reset cards
			return;
		}

		// Side Quest Button
		if (mx >= 540 && mx <= 820 && my >= 300 && my <= 380) {
			gameState = 8; // Switch to Runner Game
			RunnerGame::GetInstance().Reset();
			return;
		}

		// Back Button
		if (mx >= 20 && mx <= 140 && my >= 540 && my <= 580) {
			gameState = 10; // Return to Map
			return;
		}

		return;
	}

	// --- Memory Game Logic ---

	// Prevent interaction if game is over (Win or Loss)
	bool isGameOver = (matchesFound == 6) || (movesLeft <= 0 && lockBoard == 0);

	// Restart Button Click
	if (mx >= 860 && mx <= 980 && my >= 500 && my <= 540) {
		restartWildArea();
		return;
	}

	// Back Button Click (Goes back to selection)
	if (mx >= 20 && mx <= 140 && my >= 540 && my <= 580) {
		if (matchesFound == 6) {
			CombatManager& cm = CombatManager::GetInstance();
			cm.phase2MemoryDone = true; // Mark dynamic task as complete

			// Endgame final quest: memory done
			if (cm.allTowersCleared && !cm.endgameMemoryDone) {
				cm.endgameMemoryDone = true;
				restartWildArea();
				wildAreaMode = 0;
				if (cm.endgameRunnerDone) {
					gameState = 1; // Both done -> open outer map new!
				}
				else {
					gameState = 5; // Go back to selection
				}
				return;
			}

			// Normal: restore lives and restart tower — ONLY if this visit was caused by losing all lives
			if (cm.sentToWildAreaByLoss) {
				cm.RestoreLives();
				int tower = cm.lastTowerPlayed;
				restartWildArea();
				wildAreaMode = 0;
				if (tower == 3) {
					cm.InitTower3();
					gameState = 9;
				}
				else if (tower == 2) {
					AdvancedCombatManager::GetInstance().Init();
					gameState = 7;
				}
				else if (tower == 1) {
					cm.InitCombat();
					gameState = 6;
				}
				else if (tower == 4) {
					gameState = 1;
				}
				return;
			}

			// Voluntary visit (houses area): just go back to selection
			restartWildArea();
			wildAreaMode = 0;
			return;
		}

		// Reset Game State for next time
		restartWildArea();
		wildAreaMode = 0;
		return;
	}

	if (lockBoard != 0 || isGameOver) return;

	for (int i = 0; i < 12; i++) {
		int xPos = 180 + (i % 4) * 180;
		int yPos = 400 - (i / 4) * 150;

		if (mx >= xPos && mx <= xPos + 100 && my >= yPos && my <= yPos + 100) {
			if (cardState[i] == 0 && i != firstIdx) {
				cardState[i] = 1;
				if (firstIdx == -1) {
					firstIdx = i;
				}
				else {
					secondIdx = i;
					lockBoard = 1; // Logic handled by timer in iMain
					if (movesLeft > 0) movesLeft--; // Decrement move
				}
			}
		}
	}
}

#endif