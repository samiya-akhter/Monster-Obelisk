#ifndef WILDAREAPAGE_H
#define WILDAREAPAGE_H

extern int gameState;

// Game variables
int cards[12] = { 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6 };
int cardState[12] = { 0 }; // 0: Hidden, 1: Flipped, 2: Matched
int firstIdx = -1, secondIdx = -1;
int lockBoard = 0; // Prevents clicking during animation
int matchesFound = 0;

void shuffleCards() {
	for (int i = 11; i > 0; i--) {
		int j = rand() % (i + 1);

		int temp = cards[i];
		cards[i] = cards[j];
		cards[j] = temp;
	}
}

void drawWildArea()
{
	iShowBMP(0, 0, "Image//wildarea1.bmp");

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

	if (matchesFound == 6) {
		iSetColor(150, 0, 200);
		iText(420, 500, "YOU WIN!", GLUT_BITMAP_HELVETICA_18);
	}
}
void wildAreaClick(int mx, int my) {
	if (lockBoard != 0) return;

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
				}
			}
		}
	}
}


/*
for back icon

void backClick(int mx, int my) {
if (mx >= 427 && mx <= 558 && my >= 57 && my <= 94) {
gameState = 1;
}
}*/

#endif