#ifndef SETTINGPAGE_H
#define SETTINGPAGE_H

#include "audio.h"

extern int gameState;
extern int previousGameState;

// ----- Setting Page Draw -----
void drawSettingPage()
{
    // Draw the base settings image (already has MUSIC row, SOUND row, BACK and APPLY buttons)
    iShowImage(0, 0, 1000, 600, iLoadImage("Image//settingpage.png"));

    // Music icon area  ~ center x:605, center y:390 (music note on the right of the MUSIC row)
    // Draw red X over music icon if temp disabled
    if (!AudioSystem::tempMusicEnabled) {
        iSetColor(220, 30, 30);
        for (int t = -3; t <= 3; t++) {
            iLine(578 + t, 365, 638 + t, 415);
            iLine(638 + t, 365, 578 + t, 415);
        }
    }

    // Sound icon area  ~ center x:605, center y:300 (speaker on the right of the SOUND row)
    // Draw red X over sound icon if temp disabled
    if (!AudioSystem::tempSfxEnabled) {
        iSetColor(220, 30, 30);
        for (int t = -3; t <= 3; t++) {
            iLine(578 + t, 278, 638 + t, 328);
            iLine(638 + t, 278, 578 + t, 328);
        }
    }
}

// ----- Unified Settings Click Handler -----
// Call this from iMouse when gameState == 3
void settingPageClick(int mx, int my) {
    // Music icon toggle: approx x 578-638, y 365-415
    if (mx >= 578 && mx <= 638 && my >= 365 && my <= 415) {
        AudioSystem::tempMusicEnabled = !AudioSystem::tempMusicEnabled;
    }

    // Sound/SFX icon toggle: approx x 578-638, y 278-328
    if (mx >= 578 && mx <= 638 && my >= 278 && my <= 328) {
        AudioSystem::tempSfxEnabled = !AudioSystem::tempSfxEnabled;
    }

    // APPLY button: approx x 465-610, y 145-185
    if (mx >= 465 && mx <= 610 && my >= 145 && my <= 185) {
        // Commit temp values to real values
        AudioSystem::isMusicEnabled = AudioSystem::tempMusicEnabled;
        AudioSystem::isSfxEnabled   = AudioSystem::tempSfxEnabled;

        // Apply music change immediately
        if (!AudioSystem::isMusicEnabled) {
            mciSendString("stop bgmMain",   NULL, 0, NULL);
            mciSendString("stop bgmBattle", NULL, 0, NULL);
            mciSendString("stop bgmShop",   NULL, 0, NULL);
            mciSendString("stop bgmExtra",  NULL, 0, NULL);
            mciSendString("stop bgmWin",    NULL, 0, NULL);
            mciSendString("stop bgmLose",   NULL, 0, NULL);
        } else {
            // Force re-trigger by resetting currentBGM
            AudioSystem::currentBGM = BGM_NONE;
        }

        gameState = previousGameState;
    }

    // BACK button: approx x 310-445, y 145-185  (discard temp changes)
    if (mx >= 310 && mx <= 445 && my >= 145 && my <= 185) {
        AudioSystem::tempMusicEnabled = AudioSystem::isMusicEnabled;
        AudioSystem::tempSfxEnabled   = AudioSystem::isSfxEnabled;
        gameState = previousGameState;
    }
}

// Legacy back click (kept for any older references)
void settingBackClick(int mx, int my) {
    settingPageClick(mx, my);
}

// Settings gear icon click on main menu
void settingClick(int mx, int my) {
    if (mx >= 918 && mx <= 960 && my >= 518 && my <= 560) {
        previousGameState = gameState;
        gameState = 3;
    }
}

#endif