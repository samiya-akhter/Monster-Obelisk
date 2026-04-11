#ifndef CHARACTERSELECTION_H
#define CHARACTERSELECTION_H

#include "iGraphics.h"
#include "CombatManager.h"
#include "FinalBossManager.h"

extern int gameState; // Defined in iMain.cpp

unsigned int selectionBgTex = 0;
unsigned int viviIconIdle = 0;
unsigned int dawnIconIdle = 0;
unsigned int drakeIconIdle = 0;

void loadCharacterSelectionAssets() {
    if (selectionBgTex == 0) {
        selectionBgTex = iLoadImage("Image//monster_selection_bg.png");
        viviIconIdle = iLoadImage("Image//Monster Images//Player Monsters//Vivi//Vivi Idle Right//vivo_standing (1).png", 255, 255, 255);
        dawnIconIdle = iLoadImage("Image//Monster Images//Player Monsters//Dawn//Dawn Walk right\\frame_000.png", 255, 255, 255);
        drakeIconIdle = iLoadImage("Image//Monster Images//Player Monsters//Drake//Drake Fly right\\frame_000.png", 255, 255, 255);
    }
}

void drawCharacterSelection() {
    loadCharacterSelectionAssets();
    
    // Draw Background
    iShowImage(0, 0, 1000, 600, selectionBgTex);

    iSetColor(255, 255, 255);
    iText(230, 500, "CHOOSE YOUR MONSTER FOR THE FINAL BATTLE!", (void*)0x0006);
    iText(360, 450, "Click on a Monster to Select it", (void*)0x0008);

    // Draw Vivi (Left pedestal)
    iShowImage(150, 200, 150, 150, viviIconIdle);
    iSetColor(255, 255, 0);
    iText(200, 170, "VIVI", (void*)0x0005);

    // Draw Dawn (Center pedestal)
    if (CombatManager::GetInstance().dawnUnlocked) {
        iShowImage(425, 200, 150, 150, dawnIconIdle);
        iSetColor(0, 255, 255);
        iText(475, 170, "DAWN", (void*)0x0005);
    } else {
        // Locked
        iSetColor(100, 100, 100);
        iText(460, 250, "LOCKED", (void*)0x0005);
    }

    // Draw Drake (Right pedestal)
    if (CombatManager::GetInstance().drakeUnlocked) {
        iShowImage(700, 200, 150, 150, drakeIconIdle);
        iSetColor(255, 100, 100);
        iText(740, 170, "DRAKE", (void*)0x0005);
    } else {
        // Locked
        iSetColor(100, 100, 100);
        iText(735, 250, "LOCKED", (void*)0x0005);
    }
}

void characterSelectionClick(int mx, int my) {
    if (gameState != 12) return;

    // Check Vivi (150, 200) to (300, 350) + text area
    if (mx >= 150 && mx <= 300 && my >= 150 && my <= 350) {
        FinalBossManager::GetInstance().Init("Vivi");
        gameState = 11;
    }

    // Check Dawn
    if (CombatManager::GetInstance().dawnUnlocked) {
        if (mx >= 425 && mx <= 575 && my >= 150 && my <= 350) {
            FinalBossManager::GetInstance().Init("Dawn");
            gameState = 11;
        }
    }

    // Check Drake
    if (CombatManager::GetInstance().drakeUnlocked) {
        if (mx >= 700 && mx <= 850 && my >= 150 && my <= 350) {
            FinalBossManager::GetInstance().Init("Drake");
            gameState = 11;
        }
    }
}

#endif
