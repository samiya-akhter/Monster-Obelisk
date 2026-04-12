#ifndef NAMEINPUTPAGE_H
#define NAMEINPUTPAGE_H

#include "iGraphics.h"
#include "SaveManager.h"

extern int gameState; // 15
extern int playState; 

static unsigned int nameInputBgImg = 0;
static char inputNameBuf[50] = "";
static int inputNameLen = 0;

void drawNameInputPage() {
    if (nameInputBgImg == 0) {
        nameInputBgImg = iLoadImage("Image//name_input_bg.png");
    }
    
    // Draw background
    iShowImage(0, 0, 1000, 600, nameInputBgImg);
    
    // Dimming overlay
    iSetColor(0, 0, 0);
    iFilledRectangle(0, 0, 1000, 600); // Wait, image is opaque. Instead of black rect, let's just draw the image if it's there.
    
    // Re-draw image properly to overlap the errant black rectangle drawn above
    iShowImage(0, 0, 1000, 600, nameInputBgImg);

    // Draw Input box
    iSetColor(50, 50, 50);
    iFilledRectangle(300, 250, 400, 60);
    iSetColor(255, 215, 0); // Gold border
    iRectangle(300, 250, 400, 60);
    
    iSetColor(255, 255, 255);
    const char* promptStr = "Enter Your Name (Press ENTER to confirm):";
    int promptWidth = 41 * 9;
    iText(500 - promptWidth / 2, 330, promptStr, (void*)0x0002); // 9x15 Chunky Pixel Font
    
    // Render text
    iSetColor(255, 255, 0); // yellow text
    int textWidth = inputNameLen * 9;
    int textX = 500 - textWidth / 2;
    int textY = 250 + (60 - 15) / 2;
    iText(textX, textY, inputNameBuf, (void*)0x0002);
    
    // Cursor blink
    static int blink = 0;
    blink++;
    if ((blink / 20) % 2 == 0) {
        int cursorX = textX + textWidth + 2;
        iSetColor(255, 255, 0);
        iFilledRectangle(cursorX, textY, 9, 15);
    }
}

void nameInputKeyboardUpdate() {
    static bool keyReleased[256];
    static bool initFlags = false;
    if (!initFlags) {
        for (int i=0; i<256; i++) { keyReleased[i] = true; }
        initFlags = true;
    }

    if (isKeyPressed('\r') || isKeyPressed(13)) { 
        if (keyReleased['\r']) {
            if (inputNameLen > 0) {
                SaveManager::SaveGame(inputNameBuf);
                gameState = 1; 
                playState = 1; // Start Game (Spawn Map and Conversation)
            }
            keyReleased['\r'] = false;
        }
    } else { keyReleased['\r'] = true; }

    if (isKeyPressed('\b') || isKeyPressed(8)) {
        if (keyReleased['\b']) {
            if (inputNameLen > 0) {
                inputNameLen--;
                inputNameBuf[inputNameLen] = '\0';
            }
            keyReleased['\b'] = false;
        }
    } else { keyReleased['\b'] = true; }

    if (inputNameLen < 15) { 
        // Alphabets (lower)
        for (int k = 'a'; k <= 'z'; k++) {
            if (isKeyPressed(k)) {
                if (keyReleased[k]) {
                    inputNameBuf[inputNameLen++] = k;
                    inputNameBuf[inputNameLen] = '\0';
                    keyReleased[k] = false;
                }
            } else { keyReleased[k] = true; }
        }
        // Alphabets (upper)
        for (int k = 'A'; k <= 'Z'; k++) {
            if (isKeyPressed(k)) {
                if (keyReleased[k]) {
                    inputNameBuf[inputNameLen++] = k;
                    inputNameBuf[inputNameLen] = '\0';
                    keyReleased[k] = false;
                }
            } else { keyReleased[k] = true; }
        }
        // Numbers
        for (int k = '0'; k <= '9'; k++) {
            if (isKeyPressed(k)) {
                if (keyReleased[k]) {
                    inputNameBuf[inputNameLen++] = k;
                    inputNameBuf[inputNameLen] = '\0';
                    keyReleased[k] = false;
                }
            } else { keyReleased[k] = true; }
        }
        // Space
        if (isKeyPressed(' ')) {
            if (keyReleased[' ']) {
                inputNameBuf[inputNameLen++] = ' ';
                inputNameBuf[inputNameLen] = '\0';
                keyReleased[' '] = false;
            }
        } else { keyReleased[' '] = true; }
    }
}

#endif
