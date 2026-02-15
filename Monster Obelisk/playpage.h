#ifndef PLAYPAGE_H
#define PLAYPAGE_H
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
	"image//map1.bmp",
	"image//map2.bmp",
	"image//map3.bmp",
	"image//map4.bmp",
	"image//map5.bmp",
	"image//map6.bmp",
	"image//map7.bmp",
	"image//map8.bmp"
};

int storyFrame = 0;

int playFrame = 0;

void drawPlayPage()
{
	if (playState == 1) iShowBMP(0, 0, storySlides[storyFrame]);
	else {
		iShowBMP(0, 0, map[playFrame]);
	}
}

void updateStory()
{
	if (playState == 1) {
		storyFrame++;
		if (storyFrame >= 8) {
			playState = 2; // End story state
			gameState = 1; // Transition to Map
			storyFrame = 0; // Reset for future use
		}
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
	// Coordinates for wild area
	if (mx >= 100 && mx <= 420 && my >= 310 && my <= 520) {
		gameState = 5;
	}
	// Coordinates for Battle Tower 1
	if (mx >= 590 && mx <= 660 && my >= 230 && my <= 640) {
		gameState = 6;
	}
}
#endif
