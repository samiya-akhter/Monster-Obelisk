#ifndef SHOPPAGE_H
#define SHOPPAGE_H

#include "iGraphics.h"
#include "CombatManager.h"
#include "AdvancedCombatManager.h"
#include "RunnerGame.h"
#include "OpenWorld.h"
#include <stdio.h>

extern int gameState; // 1 = openworld, 7 = tower 2
static int shopMenuState = 0; // 0 = Selection, 1 = Shop
static unsigned int tower2SelectionImg = 0;
static unsigned int shopState1Img = 0;

static void LoadShopUI() {
    if (tower2SelectionImg == 0) tower2SelectionImg = iLoadImage("Image//tower2_selection.png");
    if (shopState1Img == 0) shopState1Img = iLoadImage("Image//shop_state1.png");
}

static void drawShopMaster() {
    LoadShopUI();
    CombatManager& cm = CombatManager::GetInstance();
    RunnerGame& rg = RunnerGame::GetInstance();

    // Solid background for the selections and shop pages
    iSetColor(0, 0, 0);
    iFilledRectangle(0, 0, 1000, 600);
    
    iSetColor(255, 255, 255);

    if (shopMenuState == 0) {
        // Selection Page
        if (tower2SelectionImg) iShowImage(0, 0, 1000, 600, tower2SelectionImg);
        
        // BACK BUTTON
        iSetColor(200, 0, 0);
        iFilledRectangle(20, 540, 80, 40);
        iSetColor(255, 255, 255);
        iText(20 + 80 / 2 - (4 * 9) / 2, 540 + 40 / 2 - 15 / 2, "BACK", (void*)0x0002);
    } 
    else if (shopMenuState == 1) {
        // Shop Page
        if (shopState1Img) iShowImage(0, 0, 1000, 600, shopState1Img);
        
        iSetColor(255, 215, 0);
        char coinStr[50];
        sprintf_s(coinStr, sizeof(coinStr), "Wallet: %d Coins", rg.GetAccumulatedCoins());
        iText(800, 550, coinStr, (void*)0x0002);

        // Draw Heal Potion if Tower 1 is cleared
        if (cm.tower1Cleared) {
            // We use iText overlay in case generation was flawed or missing details
            iSetColor(255, 255, 255);
            iText(220, 220, "BUY: 50 Coins", (void*)0x0002);
            char inv1[50];
            sprintf_s(inv1, sizeof(inv1), "Owned: %d", cm.healPotionCount);
            iText(220, 170, inv1, (void*)0x0002);
        }

        // Draw Damage Potion if Tower 1 AND Tower 3 are cleared
        if (cm.tower1Cleared && cm.tower3Cleared) {
            iSetColor(150, 50, 255);
            iFilledRectangle(500, 200, 200, 250); // overlay a purple rect for damage potion
            iSetColor(255, 255, 255);
            iText(500 + 200 / 2 - (13 * 9) / 2, 420, "DAMAGE POTION", (void*)0x0002); // 13 chars
            iText(500 + 200 / 2 - (13 * 9) / 2, 220, "BUY: 50 Coins", (void*)0x0002);

            char inv2[50];
            sprintf_s(inv2, sizeof(inv2), "Owned: %d", cm.damagePotionCount);
            iText(500 + 200 / 2 - (13 * 9) / 2, 170, inv2, (void*)0x0002);
        }

        // BACK BUTTON
        iSetColor(200, 0, 0);
        iFilledRectangle(20, 540, 80, 40);
        iSetColor(255, 255, 255);
        iText(20 + 80 / 2 - (4 * 9) / 2, 540 + 40 / 2 - 15 / 2, "BACK", (void*)0x0002);
    }
}

static void iMouseShop(int button, int state, int mx, int my) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        CombatManager& cm = CombatManager::GetInstance();
        RunnerGame& rg = RunnerGame::GetInstance();

        if (shopMenuState == 0) {
            // Select Tower 2 vs Shop (using approximate large hitboxes for the generated UI)
            if (mx >= 100 && mx <= 450 && my >= 100 && my <= 450) { // left side rect
                shopMenuState = 0;
                AdvancedCombatManager::GetInstance().Init();
                gameState = 7;
            }
            if (mx >= 550 && mx <= 900 && my >= 100 && my <= 450) { // right side rect
                shopMenuState = 1;
            }
            // Back button
            if (mx >= 20 && mx <= 100 && my >= 540 && my <= 580) {
                shopMenuState = 0;
                gameState = 10; // Back to Open World Spawn Map
                OpenWorldGame::GetInstance().SetTriggerCooldown(1.5f);
            }
        } 
        else if (shopMenuState == 1) {
            // Buy Heal Potion (Left slot)
            if (cm.tower1Cleared) {
                if (mx >= 150 && mx <= 450 && my >= 200 && my <= 450) {
                    if (rg.GetAccumulatedCoins() >= 50) {
                        rg.AddAccumulatedCoins(-50);
                        cm.healPotionCount++;
                    }
                }
            }
            // Buy Damage Potion (Right slot)
            if (cm.tower1Cleared && cm.tower3Cleared) {
                if (mx >= 500 && mx <= 700 && my >= 200 && my <= 450) {
                    if (rg.GetAccumulatedCoins() >= 50) {
                        rg.AddAccumulatedCoins(-50);
                        cm.damagePotionCount++;
                    }
                }
            }
            // Back button
            if (mx >= 20 && mx <= 100 && my >= 540 && my <= 580) {
                shopMenuState = 0; // Go back to Selection
            }
        }
    }
}

#endif // SHOPPAGE_H
