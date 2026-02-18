#ifndef SETTINGPAGE_H
#define SETTINGPAGE_H


// Music state
int musicOn = 1;

void drawSettingPage()
{
	iShowImage(0, 0,1000,600,iLoadImage( "Image//settingpage.png"));

    
   
}

void settingClick(int mx, int my) {
	if (mx >= 918 && mx <= 960 && my >= 518 && my <= 560) {
		gameState = 3;
        // Toggle Music (400, 300, 200, 50)
        if (mx >= 400 && mx <= 600 && my >= 300 && my <= 350) {
            musicOn = !musicOn;
            if (musicOn) {
				mciSendString("play mainMusic from 0 repeat", NULL, 0, NULL);
            } else {
				mciSendString("stop mainMusic", NULL, 0, NULL);
            }
        }
    }
}




#endif