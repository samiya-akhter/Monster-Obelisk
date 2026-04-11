#ifndef EISHIRO_CONVERSATION_H
#define EISHIRO_CONVERSATION_H

#include "iGraphics.h"
#include "OpenWorld.h"

extern int gameState;

// ---------------------------------------------------------------
// Eishiro conversation — shown on top of the open world map
// after the story sequence completes.
// Each line: { speaker 0=Eishiro 1=Kael, text }
// ---------------------------------------------------------------

struct ConvLine {
    int  speaker;   // 0 = Eishiro, 1 = Kael
    const char* text;
};

static ConvLine convLines[] = {
    { 0, "Hold it. You're Kael, right? Aris told me you were coming." },
    { 0, "I'm Eishiro, and I don't like my time being wasted." },
    { 1, "I'm ready to help. What's the mission?" },
    { 0, "The path to saving Eternia starts at Tower 1. Your task is" },
    { 0, "simple: enter the stronghold and defeat every monster inside." },
    { 0, "Clean sweep, no exceptions." },
    { 1, "Understood. Vivi and I are on it." },
    { 0, "Your Tamer ID tracks your vitals - you have exactly 3 lives." },
    { 0, "If you're defeated three times, you're out. You'll have to" },
    { 0, "retreat and restore your life before you can try again." },
    { 1, "Clear the tower, watch my 3 lives. Anything else?" },
    { 0, "you can view tasks by clicking on the task icon. And...just stay alive. Now get moving." }
};

static const int CONV_TOTAL = 12;

// State
static int  convIndex      = 0;
static bool convDone       = false;
static bool convInitialized = false;

// Loaded portrait textures (loaded once)
static unsigned int eishiroTex = 0;
static unsigned int kaelTex    = 0;

// ---------------------------------------------------------------
// Helper: wrap text to fit inside maxWidth pixels (approx 8px/ch)
// Returns number of lines used.
// ---------------------------------------------------------------
static void DrawWrappedText(int x, int y, const char* text, int maxWidth, void* font)
{
    char buf[512];
    int  bufLen = 0;
    int  line_y = y;
    int  charW  = 10; // More conservative width estimation to prevent overflow
    int  lineHeight = 22;

    int i = 0;
    while (text[i] != '\0') {
        // Read next word
        int wordStart = i;
        while (text[i] != '\0' && text[i] != ' ') i++;
        int wordLen = i - wordStart;

        // Check if word fits in current buffer
        if ((bufLen + wordLen + 1) * charW < maxWidth) {
            // Append word
            if (bufLen > 0) buf[bufLen++] = ' ';
            for (int k = 0; k < wordLen; k++) buf[bufLen++] = text[wordStart + k];
            buf[bufLen] = '\0';
        } else {
            // Flush current buffer
            if (bufLen > 0) {
                iText(x, line_y, buf, font);
                line_y -= lineHeight;
            }
            // Start new line with this word
            bufLen = 0;
            for (int k = 0; k < wordLen; k++) buf[bufLen++] = text[wordStart + k];
            buf[bufLen] = '\0';
        }

        // Skip space
        if (text[i] == ' ') i++;
    }

    // Final buffer
    if (bufLen > 0) {
        iText(x, line_y, buf, font);
    }
}

// ---------------------------------------------------------------
void InitEishiroConversation()
{
    convIndex = 0;
    convDone  = false;

    if (!convInitialized) {
        // Load portraits using iGraphics loader
        eishiroTex = iLoadImage("Image/Eishiro.png");
        kaelTex    = iLoadImage("Image/Kael.png");
        convInitialized = true;
    }

    // Show the open-world map underneath from the house spawn
    OpenWorldGame::GetInstance().Init();   // resets player to default pos
}

// ---------------------------------------------------------------
// Returns true once the conversation has finished (caller should
// switch game state).
// ---------------------------------------------------------------
bool IsConversationDone() { return convDone; }

// ---------------------------------------------------------------
// Advance to next dialogue line (call on SPACE press).
// ---------------------------------------------------------------
void AdvanceConversation()
{
    if (convDone) return;
    convIndex++;
    if (convIndex >= CONV_TOTAL)
        convDone = true;
}

// ---------------------------------------------------------------
// Render the conversation overlay on top of the open-world map.
// ---------------------------------------------------------------
void RenderEishiroConversation()
{
    // Safety check: ensure images are loaded
    if (!convInitialized || eishiroTex == 0) {
        InitEishiroConversation();
    }

    // --- draw the open-world map in the background (static, no player yet) ---
    OpenWorldGame::GetInstance().Render(false);

    if (convDone) return;

    ConvLine& cl = convLines[convIndex];

    // Layout constants
    const int BOX_W    = 750;
    const int BOX_H    = 160;
    const int BOX_X    = (1000 - BOX_W) / 2;   // horizontally centred
    const int BOX_Y    = 30;                     // near bottom
    const int ICON_SZ  = 130;
    const int PAD      = 15;

    // Semi-transparent dark panel background
    iSetColor(15, 15, 30);
    iFilledRectangle(BOX_X - 2, BOX_Y - 2, BOX_W + 4, BOX_H + 4);

    // Coloured border: blue for Eishiro, gold for Kael
    if (cl.speaker == 0)
        iSetColor(80, 140, 220);
    else
        iSetColor(220, 180, 60);
    iRectangle(BOX_X - 2, BOX_Y - 2, BOX_W + 4, BOX_H + 4);
    iRectangle(BOX_X - 1, BOX_Y - 1, BOX_W + 2, BOX_H + 2);

    // Flat dark box
    iSetColor(22, 22, 45);
    iFilledRectangle(BOX_X, BOX_Y, BOX_W, BOX_H);

    // Portrait icon area
    int iconX = BOX_X + PAD;
    int iconY = BOX_Y + (BOX_H - ICON_SZ) / 2;

    // Icon background circle/square
    if (cl.speaker == 0)
        iSetColor(40, 70, 120);
    else
        iSetColor(100, 80, 20);
    iFilledRectangle(iconX, iconY, ICON_SZ, ICON_SZ);

    // Draw portrait
    unsigned int tex = (cl.speaker == 0) ? eishiroTex : kaelTex;
    if (tex)
        iShowImage(iconX, iconY, ICON_SZ, ICON_SZ, tex);

    // Icon border
    if (cl.speaker == 0)
        iSetColor(80, 140, 220);
    else
        iSetColor(220, 180, 60);
    iRectangle(iconX, iconY, ICON_SZ, ICON_SZ);

    // Speaker name badge
    int nameX = iconX;
    int nameY = iconY + ICON_SZ + 2;
    iSetColor(30, 30, 55);
    iFilledRectangle(nameX, nameY, ICON_SZ, 20);
    if (cl.speaker == 0)
        iSetColor(100, 170, 255);
    else
        iSetColor(255, 210, 80);
    iText(nameX + 4, nameY + 4,
          (cl.speaker == 0) ? "Eishiro" : "Kael",
          (void*)0x0005);

    // Dialogue text area
    int textX = iconX + ICON_SZ + PAD * 2;
    int textW = BOX_W - ICON_SZ - PAD * 4;
    int textY = BOX_Y + BOX_H - 35;

    iSetColor(220, 220, 240);
    DrawWrappedText(textX, textY, cl.text, textW, (void*)0x0005);

    // Progress indicator dots
    int dotBaseX = BOX_X + BOX_W / 2 - (CONV_TOTAL * 10) / 2;
    int dotY_pos = BOX_Y + BOX_H + 10;
    for (int i = 0; i < CONV_TOTAL; i++) {
        if (i == convIndex)
            iSetColor(255, 255, 255);
        else
            iSetColor(70, 70, 90);
        iFilledCircle(dotBaseX + i * 10, dotY_pos, 3);
    }

    // SPACE prompt (bottom-right of box)
    iSetColor(120, 120, 140);
    iText(BOX_X + BOX_W - 150, BOX_Y + 8, "SPACE to continue", (void*)0x0004);
}

#endif // EISHIRO_CONVERSATION_H
