#ifndef PLAYPAGE_H
#define PLAYPAGE_H
#include "AdvancedCombatManager.h"
#include "CombatManager.h"
#include "OpenWorld.h"

extern int playState;
extern int gameState;

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

char map[8][25] = {
	"image//playpage1.png", "image//playpage2.png",
	"image//playpage3.png", "image//playpage4.png",
	"image//playpage5.png", "image//playpage6.png",
	"image//playpage7.png", "image//playpage8.png"
};

int storyFrame = 0;
int playFrame = 0;

// Button coordinates
int skipX = 850, skipY = 550, skipW = 100, skipH = 40;

void drawPlayPage() {
  if (playState == 1) {
    iShowImage(0, 0, 1000, 600, iLoadImage(storySlides[storyFrame]));

    if (storyFrame < 7) {
        // Skip Button
        iSetColor(0, 0, 0);
        iFilledRectangle(skipX, skipY, skipW, skipH);
        iSetColor(255, 255, 255);
        iText(skipX + 30, skipY + 15, "SKIP", (void *)0x0008);

        // Instruction
        iText(300, 565, "Press SPACE to Continue!", (void *)0x0008);
    } else {
        iSetColor(255, 255, 0);
        iText(340, 565, "Press SPACE to Continue!", (void *)0x0008);
    }
  } else {
    CombatManager& cm = CombatManager::GetInstance();
    if (cm.allTowersCleared && cm.endgameRunnerDone && cm.endgameMemoryDone) {
        iShowImage(0, 0, 1000, 600, iLoadImage("Image//outer map new.png"));
        iSetColor(255, 215, 0);
        iText(320, 30, "All Quests Complete! Click the Top Tower!", (void*)0x0005);
    } else {
        iShowImage(0, 0, 1000, 600, iLoadImage(map[playFrame]));
    }

    // Show "No Lives" warning if player cannot fight
    if (CombatManager::GetInstance().lives <= 0) {
      iSetColor(255, 0, 0);
      iText(320, 580, "NO LIVES! Win Memory Game in Wild Area!",
            (void *)0x0005);
    }
  }
}

void nextStorySlide() {
  if (storyFrame >= 7) {
    playState = 2; // End story state
    extern void InitEishiroConversation();
    InitEishiroConversation();
    gameState = 13; // Eishiro conversation override
    storyFrame = 0; // Reset for future use
  } else {
    storyFrame++;
  }
}

void skipStory() {
  storyFrame = 7;
}

void handleStoryClick(int mx, int my) {
  if (storyFrame < 7) {
    if (mx >= skipX && mx <= skipX + skipW && my >= skipY &&
        my <= skipY + skipH) {
      skipStory();
    }
  }
}

void updateStory() {
  // Auto-increment removed for manual control
}

void updatePlayPage() {
    playFrame++;
    if (playFrame >= 8) playFrame = 0;
}

void mapClick(int mx, int my) {
  CombatManager& cm = CombatManager::GetInstance();

  // Coordinates for Right Tower / Open World
  if (mx >= 790 && mx <= 850 && my >= 220 && my <= 360) {
    OpenWorldGame::GetInstance().Init();
    gameState = 10;
    return;
  }

  // Top Tower (Mogambo Boss) - enabled only when endquests are done
  if (cm.allTowersCleared && cm.endgameRunnerDone && cm.endgameMemoryDone) {
      // Estimated Top Tower Hitbox (~ x:420-550, y:400-550)
      if (mx >= 420 && mx <= 550 && my >= 400 && my <= 550) {
          gameState = 12; // Character Selection
          return;
      }
  }
}
#endif
