#ifndef STORYPAGE_H
#define STORYPAGE_H
#include "AdvancedCombatManager.h"
#include "OpenWorld.h"
#include "CombatManager.h"
extern int playState;
extern int gameState;

// Updated to use the new PNG story images (story (1).png ... story (8).png)
char storySlides[8][40] = {
    "Image//story (1).png",
    "Image//story (2).png",
    "Image//story (3).png",
    "Image//story (4).png",
    "Image//story (5).png",
    "Image//story (6).png",
    "Image//story (7).png",
    "Image//story (8).png"
};

int storyFrame = 0;

// Button coordinates
int skipX = 850, skipY = 550, skipW = 100, skipH = 40;

void drawStoryPage()
{
    if (playState == 1) {
        iShowImage(0, 0, 1000, 600, iLoadImage(storySlides[storyFrame]));

        // Only show SKIP button on slides 1-7 (not the last slide)
        if (storyFrame < 7) {
            // Bright red SKIP button (top-right)
            iSetColor(0, 0, 0);
            iFilledRectangle(skipX, skipY, skipW, skipH);
            iSetColor(255, 255, 255);
            iText(skipX + 30, skipY + 15, "SKIP", (void*)0x0008);
        }

        // Top instruction text
        if (storyFrame < 7) {
            iSetColor(255, 255, 255);
            iText(300, 565, "Press SPACE to Continue!", (void*)0x0008);
        } else {
            // On the last slide (story 8), prompt to go to open world
            iSetColor(255, 255, 0);
            iText(340, 565, "Press SPACE to Continue!", (void*)0x0008);
        }
    }
}

void goToEishiroConversation() {
    playState = 2;
    // Initialize conversation and load images
    InitEishiroConversation();
    gameState = 11;  // Eishiro conversation overlay
    storyFrame = 0;
}

void nextStorySlide() {
    if (storyFrame >= 7) {
        // Last slide: SPACE goes to Eishiro conversation
        goToEishiroConversation();
    } else {
        storyFrame++;
    }
}

void skipStory() {
    // Skip jumps to slide 8 (index 7), the last story slide
    storyFrame = 7;
}

void handleStoryClick(int mx, int my) {
    // Only handle skip click if not already on last slide
    if (storyFrame < 7) {
        if (mx >= skipX && mx <= skipX + skipW && my >= skipY && my <= skipY + skipH) {
            skipStory();
        }
    }
}

void updateStory()
{
    // Auto-increment removed for manual control
}

#endif
