#ifndef WILDAREAPAGE_H
#define WILDAREAPAGE_H

extern int gameState;

// Game variables
int cards[12] = { 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6 };
int cardState[12] = { 0 }; // 0: Hidden, 1: Flipped, 2: Matched
int firstIdx = -1, secondIdx = -1;
int lockBoard = 0; // Prevents clicking during animation
int matchesFound = 0;
int movesLeft = 10;
const int MAX_MOVES = 12;

void shuffleCards() {
    // Reset game state
    matchesFound = 0;
    movesLeft = MAX_MOVES;
    firstIdx = -1;
    secondIdx = -1;
    lockBoard = 0;
    for(int k=0; k<12; k++) cardState[k] = 0;

	for (int i = 11; i > 0; i--) {
		int j = rand() % (i + 1);

		int temp = cards[i];
		cards[i] = cards[j];
		cards[j] = temp;
	}
}

void restartWildArea() {
    shuffleCards();
}

void drawWildArea()
{
	iShowBMP(0, 0, "Image//wildarea1.bmp");

    // Game Title
    iSetColor(255, 255, 255);
    iText(400, 580, "Wild Area: Memory Challenge", (void*)0x0005);

    // Moves Left
    char movesBuf[32];
    sprintf_s(movesBuf, 32, "Moves Left: %d", movesLeft);
    if (movesLeft <= 3) iSetColor(255, 0, 0); // Warning color
    else iSetColor(255, 255, 255);
    iText(400, 550, movesBuf, (void*)0x0005);

    // Pairs Found
    char pairsBuf[32];
    sprintf_s(pairsBuf, 32, "Pairs Found: %d/6", matchesFound);
    iSetColor(255, 255, 255);
    iText(400, 520, pairsBuf, (void*)0x0005);

	for (int i = 0; i < 12; i++) {
		int xPos = 180 + (i % 4) * 180;
		int yPos = 400 - (i / 4) * 150;

		if (cardState[i] == 0) {
			iShowBMP2(xPos, yPos, "Image//card_back.bmp",0);
		}
		else if(cardState[i] == 1) {
			char cardImg[30];
			sprintf_s(cardImg, sizeof(cardImg), "Image//poke%d.bmp", cards[i]);
			iShowBMP2(xPos, yPos, cardImg,0);
		}
		else if (cardState[i] == 2){
			continue;
		}
	}

    // Back Button
    iSetColor(150, 150, 150);
    iFilledRectangle(20, 540, 80, 40);
    iSetColor(255, 255, 255);
    iText(35, 555, "BACK", (void*)0x0008);

    // Restart Button
    iSetColor(150, 150, 150);
    iFilledRectangle(900, 500, 90, 40);
    iSetColor(255, 255, 255);
    iText(910, 515, "RESTART", (void*)0x0008);

    // Win State
	if (matchesFound == 6) {
		iSetColor(150, 0, 200);
		iShowBMP2(0, 0, "Image//fullenergy.bmp",0);
        
        // Visual confirmation
        iSetColor(0, 255, 0);
        iText(300, 100, "Victory! Lives Restored.", (void*)0x0005);
	}
    // Loss State
    else if (movesLeft <= 0 && lockBoard == 0) { // Check lockBoard so we don't show loss while last pair is checking
        iSetColor(255, 0, 0);
        iText(200, 100, "You failed! Try again to restore your monster's lives", (void*)0x0005);
    }

    // Show current lives
    char livesBuf[32];
    sprintf_s(livesBuf, 32, "Lives: %d", CombatManager::GetInstance().lives);
    iSetColor(255, 255, 255);
    iText(900, 560, livesBuf, (void*)0x0008);
}

void wildAreaClick(int mx, int my) {
    // Prevent interaction if game is over (Win or Loss)
    bool isGameOver = (matchesFound == 6) || (movesLeft <= 0 && lockBoard == 0);

    // Restart Button Click
    if (mx >= 900 && mx <= 990 && my >= 500 && my <= 540) {
        restartWildArea();
        return;
    }

    // Back Button Click
    if (mx >= 20 && mx <= 100 && my >= 540 && my <= 580) {
        if (matchesFound == 6) {
            CombatManager::GetInstance().AddStrengthBonus(10.0f); // Award bonus
            CombatManager::GetInstance().RestoreLives();          // Restore lives!
        }
        
        // Reset Game State for next time
        restartWildArea(); // Use restart to clean up
        
        gameState = 1; // Back to map
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
