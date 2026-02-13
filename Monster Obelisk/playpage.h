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

#endif
