#ifndef PLAYPAGE_H
#define PLAYPAGE_H

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

int playFrame = 0;

void drawPlayPage()
{
	iShowBMP(0, 0, map[playFrame]);
}

void updatePlayPage()
{
	playFrame++;
	if (playFrame >= 8)
		playFrame = 0;
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
