#ifndef OPENWORLD_H
#define OPENWORLD_H

#include "iGraphics.h"
#include <math.h>
#include <stdio.h>
#include <windows.h>
#include "CombatManager.h"
#include "RunnerGame.h"

extern "C" {
    unsigned char *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp);
    void stbi_image_free(void *retval_from_stbi_load);
}

#define OW_SCREEN_WIDTH  1000
#define OW_SCREEN_HEIGHT 600
#define OW_WORLD_WIDTH   1500
#define OW_WORLD_HEIGHT  1000
#define OW_PLAYER_SPEED  150.0
#define OW_RUN_MULT      1.8
#define OW_ZOOM          2.5
#define OW_NUM_FRAMES    9

// Minimap
#define OW_MM_X          10
#define OW_MM_Y          10
#define OW_MM_W          200
#define OW_MM_H          133
#define OW_MM_PAD        3

struct OWPlayer { double x, y; };

class OpenWorldGame {
public:
    static OpenWorldGame& GetInstance() {
        static OpenWorldGame inst;
        return inst;
    }

    unsigned int LoadHugeImage(const char* filename) {
        int w, h, ch;
        unsigned char* data = stbi_load(filename, &w, &h, &ch, 4);
        if (!data) {
            printf("[OpenWorld] FAILED to load image: %s\n", filename);
            return 0;
        }
        unsigned int tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
        printf("[OpenWorld] Loaded image: %s (%dx%d)\n", filename, w, h);
        return tex;
    }

    void Init() {
        player.x = 810.0;
        player.y = 115.0;

        if (initialized) {
            printf("[OpenWorld] Already initialized, just reset position.\n");
            return;
        }

        printf("[OpenWorld] Initializing tile system...\n");

        // Load task icon ignoring black background (0,0,0)
        taskIcon = iLoadImage("Image/task.png", 0, 0, 0);
        // Phase task images (transparent PNG boards)
        taskPhaseImages[0] = iLoadImage("Image/task_phase1.png", 0, 0, 0); // Phase 1: Tower1, 10 coins, Dawn
        taskPhaseImages[1] = iLoadImage("Image/task_phase2.png", 0, 0, 0); // Phase 2: Tower2, 20 coins, Memory
        taskPhaseImages[2] = iLoadImage("Image/task_phase3.png", 0, 0, 0); // Phase 3: Tower3, Drake
        taskPhaseImages[3] = iLoadImage("Image/task_phase4.png", 0, 0, 0); // Phase 4: Final Quest
        showTask1 = false;

        char path[256];
        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 5; c++) {
                sprintf_s(path, sizeof(path), "Image\\openworld map\\openworld_map [%d][%d].png", r + 1, c + 1);
                bgTiles[r][c] = LoadHugeImage(path);

                sprintf_s(path, sizeof(path), "Image\\maskmap\\maskmap [%d][%d].png", r + 1, c + 1);
                int ch = 0;
                maskTiles[r][c] = stbi_load(path, &maskW_tile[r][c], &maskH_tile[r][c], &ch, 3);
                
                if (maskTiles[r][c]) {
                    printf("[OpenWorld] Mask map loaded: [%d][%d] (%dx%d)\n", r + 1, c + 1, maskW_tile[r][c], maskH_tile[r][c]);
                } else {
                    printf("[OpenWorld] ERROR: Could not load mask map for tile [%d][%d]\n", r + 1, c + 1);
                }
            }
        }

        const char* dirs[4] = { "down", "up", "left", "right" };
        for (int d = 0; d < 4; d++) {
            for (int f = 0; f < OW_NUM_FRAMES; f++) {
                int fn = (d == 3 && f == 8) ? 9 : f;
                sprintf_s(path, sizeof(path), "Image\\player-%s-%d.png", dirs[d], fn);
                playerFrames[d][f] = iLoadImage(path);
            }
        }

        initialized = true;
    }

    void SetSpawnPosition(double x, double y) {
        player.x = x;
        player.y = y;
    }

    void HandleMouseClick(int mx, int my) {
        // Task icon button area (bottom-right)
        int btnX = OW_SCREEN_WIDTH - 120;
        int btnY = 20;
        int btnW = 100;
        int btnH = 100;

        if (mx >= btnX && mx <= btnX + btnW && my >= btnY && my <= btnY + btnH) {
            showTask1 = !showTask1;
            printf("[OpenWorld] Task Icon clicked. showTask1 = %d\n", showTask1);
        } else if (showTask1) {
            // If interface is open, clicking anywhere else/inside can close it
            showTask1 = false;
        }
    }

    void SetTriggerCooldown(double seconds) {
        triggerCooldown = seconds;
    }

    void UpdateAnimation() {
        if (isMoving) {
            animFrame = (animFrame + 1) % OW_NUM_FRAMES;
        } else {
            animFrame = 0;
        }
    }

    bool GetMaskPixel(double x, double y, unsigned char& R, unsigned char& G, unsigned char& B) {
        int c = (int)(x / (OW_WORLD_WIDTH / 5.0));
        int r = (int)((OW_WORLD_HEIGHT - y) / (OW_WORLD_HEIGHT / 3.0));
        
        if (c < 0) c = 0; if (c > 4) c = 4;
        if (r < 0) r = 0; if (r > 2) r = 2;
        
        if (!maskTiles[r][c]) {
            R = G = B = 255;
            return false;
        }

        double tileStartX = c * (OW_WORLD_WIDTH / 5.0);
        double tileTopY = OW_WORLD_HEIGHT - r * (OW_WORLD_HEIGHT / 3.0);

        double relX = x - tileStartX;
        double relY = tileTopY - y;

        int tilePixW = maskW_tile[r][c];
        int tilePixH = maskH_tile[r][c];

        int mx = (int)(relX * tilePixW / (OW_WORLD_WIDTH / 5.0));
        int my = (int)(relY * tilePixH / (OW_WORLD_HEIGHT / 3.0));

        if (mx < 0) mx = 0; if (mx >= tilePixW) mx = tilePixW - 1;
        if (my < 0) my = 0; if (my >= tilePixH) my = tilePixH - 1;

        int idx = (my * tilePixW + mx) * 3;
        R = maskTiles[r][c][idx];
        G = maskTiles[r][c][idx+1];
        B = maskTiles[r][c][idx+2];
        return true;
    }

    bool CheckForBattleTower1Trigger() {
        if (triggerCooldown > 0.0) return false;
        unsigned char r, g, b;
        if (!GetMaskPixel(player.x, player.y, r, g, b)) return false;
        return (r > 100 && g < 100 && b < 100);
    }

    bool CheckForBattleTower2Trigger() {
        if (triggerCooldown > 0.0) return false;
        unsigned char r, g, b;
        if (!GetMaskPixel(player.x, player.y, r, g, b)) return false;
        return (g > 100 && r < 100 && b < 100);
    }

    bool CheckForWildAreaTrigger() {
        if (triggerCooldown > 0.0) return false;
        // The user specified the wild area starts at the top left map portion
        return (player.x < 350.0 && player.y > 750.0);
    }

    bool CheckForHousesTrigger() {
        return (player.x < 350.0 && player.y > 750.0);
    }

    bool CheckForBattleTower3Trigger() {
        if (triggerCooldown > 0.0) return false;
        unsigned char r, g, b;
        if (!GetMaskPixel(player.x, player.y, r, g, b)) return false;
        return (r > 100 && g > 100 && b < 100);
    }

    bool IsWall(double x, double y) {
        unsigned char r_col, g_col, b_col;
        if (!GetMaskPixel(x, y, r_col, g_col, b_col)) return false;

        bool onPath       = (r_col < 60 && g_col < 60 && b_col < 60);
        bool onBattleTower2Zone = (g_col > 100 && r_col < 100 && b_col < 100);
        bool onWildAreaZone = (player.x < 350.0 && player.y > 750.0) || (b_col > 100 && r_col < 100 && g_col < 100); // Kept blue walkability just in case
        bool onBattleTower1Zone = (r_col > 100 && g_col < 100 && b_col < 100);
        bool onBattleTower3Zone = (r_col > 100 && g_col > 100 && b_col < 100);

        return !(onPath || onBattleTower2Zone || onWildAreaZone || onBattleTower1Zone || onBattleTower3Zone);
    }

    void Update(double dt) {
        if (triggerCooldown > 0.0) {
            triggerCooldown -= dt;
        }

        double speed = OW_PLAYER_SPEED;
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
            speed *= OW_RUN_MULT;

        double dx = 0, dy = 0;
        isMoving = false;

        if (isSpecialKeyPressed(GLUT_KEY_UP)    || isKeyPressed('w') || isKeyPressed('W')) { dy += speed * dt; dir = 1; isMoving = true; }
        if (isSpecialKeyPressed(GLUT_KEY_DOWN)  || isKeyPressed('s') || isKeyPressed('S')) { dy -= speed * dt; dir = 0; isMoving = true; }
        if (isSpecialKeyPressed(GLUT_KEY_LEFT)  || isKeyPressed('a') || isKeyPressed('A')) { dx -= speed * dt; dir = 2; isMoving = true; }
        if (isSpecialKeyPressed(GLUT_KEY_RIGHT) || isKeyPressed('d') || isKeyPressed('D')) { dx += speed * dt; dir = 3; isMoving = true; }

        double nx = player.x + dx;
        double ny = player.y + dy;

        if (!IsWall(nx, player.y)) player.x = nx;
        if (!IsWall(player.x, ny)) player.y = ny;

        if (player.x < 0)               player.x = 0;
        if (player.y < 0)               player.y = 0;
        if (player.x > OW_WORLD_WIDTH)  player.x = OW_WORLD_WIDTH;
        if (player.y > OW_WORLD_HEIGHT) player.y = OW_WORLD_HEIGHT;

        double halfW = (OW_SCREEN_WIDTH  / OW_ZOOM) / 2.0;
        double halfH = (OW_SCREEN_HEIGHT / OW_ZOOM) / 2.0;
        camX = player.x - halfW;
        camY = player.y - halfH;
        if (camX < 0) camX = 0;
        if (camY < 0) camY = 0;
        if (camX > OW_WORLD_WIDTH  - halfW * 2) camX = OW_WORLD_WIDTH  - halfW * 2;
        if (camY > OW_WORLD_HEIGHT - halfH * 2) camY = OW_WORLD_HEIGHT - halfH * 2;
    }

    void Render(bool showMinimap = true) {
        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 5; c++) {
                if (!bgTiles[r][c]) continue;
                double wX = c * (OW_WORLD_WIDTH / 5.0);
                double wY = OW_WORLD_HEIGHT - (r + 1) * (OW_WORLD_HEIGHT / 3.0);
                double wW = (OW_WORLD_WIDTH / 5.0);
                double wH = (OW_WORLD_HEIGHT / 3.0);
                
                iShowImage(
                    (int)((wX - camX) * OW_ZOOM),
                    (int)((wY - camY) * OW_ZOOM),
                    (int)(ceil(wW * OW_ZOOM) + 1),
                    (int)(ceil(wH * OW_ZOOM) + 1),
                    bgTiles[r][c]);
            }
        }

        double sz = 32.0 * OW_ZOOM;
        iShowImage(
            (int)((player.x - camX) * OW_ZOOM - sz / 2.0),
            (int)((player.y - camY) * OW_ZOOM - sz / 2.0),
            (int)sz, (int)sz,
            playerFrames[dir][animFrame]);

        if (CheckForBattleTower3Trigger()) {
            iSetColor(255, 255, 0);
            iText(300, 550, "Press 'Enter' to enter Tower - 3", (void*)0x0005);
        }

        if (CheckForBattleTower2Trigger()) {
            iSetColor(0, 255, 0);
            iText(300, 550, "Press 'Enter' to enter Tower - 2", (void*)0x0005);
        }
        
        if (CheckForBattleTower1Trigger()) {
            iSetColor(255, 0, 0);
            iText(300, 550, "Press 'Enter' to enter Tower - 1", (void*)0x0005);
        }

        if (CheckForWildAreaTrigger()) {
            iSetColor(0, 255, 255);
            iText(300, 550, "Press 'Enter' to enter the Wild Area", (void*)0x0005);
        }

        // Task icon at bottom-right corner
        if (taskIcon) {
            iShowImage(OW_SCREEN_WIDTH - 120, 20, 100, 100, taskIcon);
        }

        // Task Interface (overlay) — show the image for the current phase
        if (showTask1) {
            CombatManager& cm = CombatManager::GetInstance();
            RunnerGame& rg = RunnerGame::GetInstance();

            // Determine which phase image to show
            int phaseIdx = 0;
            if (!cm.phase1RewardGiven)       phaseIdx = 0;
            else if (!cm.phase2RewardGiven)  phaseIdx = 1;
            else if (!cm.phase3RewardGiven)  phaseIdx = 2;
            else                             phaseIdx = 3;

            if (taskPhaseImages[phaseIdx]) {
                iShowImage(100, 90, 800, 420, taskPhaseImages[phaseIdx]);
            }

            // Phase completion checks & rewards
            if (!cm.phase1RewardGiven) {
                // Phase 1: Tower 1 cleared, 10 coins, Dawn unlocked
                if (cm.tower1Cleared && rg.GetAccumulatedCoins() >= 10 && cm.dawnUnlocked) {
                    cm.phase1RewardGiven = true;
                    rg.AddAccumulatedCoins(100);
                    cm.phase2CoinTarget = rg.GetAccumulatedCoins() + 20;
                }
            } else if (!cm.phase2RewardGiven) {
                // Phase 2: Tower 2 cleared, 20 more coins, memory done
                if (cm.tower2Cleared && rg.GetAccumulatedCoins() >= cm.phase2CoinTarget && cm.phase2MemoryDone) {
                    cm.phase2RewardGiven = true;
                    rg.AddAccumulatedCoins(100);
                }
            } else if (!cm.phase3RewardGiven) {
                // Phase 3: Tower 3 cleared, Drake unlocked
                if (cm.tower3Cleared && cm.drakeUnlocked) {
                    cm.phase3RewardGiven = true;
                    rg.AddAccumulatedCoins(100);
                }
            }
        }

        // Endgame prompts based on current project unlocks!
        {
            CombatManager& cm = CombatManager::GetInstance();
            if (cm.allTowersCleared) {
                bool endgameDone = cm.endgameRunnerDone && cm.endgameMemoryDone;
                if (!endgameDone) {
                    iSetColor(255, 215, 0);
                    iText(150, 35, "All towers cleared! Head to the top-left corner near the houses", (void*)0x0008);
                    iText(150, 15, "to complete the Runner and Memory games!", (void*)0x0008);
                }
                if (CheckForHousesTrigger() && !endgameDone) {
                    iSetColor(200, 200, 255);
                    iText(280, 550, "Press 'Enter' to start your final quest!", (void*)0x0005);
                }
            }
        }

        if (showMinimap) {
            RenderMinimap();
        }
    }

    void RenderMinimap() {
        iSetColor(0, 0, 0);
        iFilledRectangle(OW_MM_X - OW_MM_PAD,
                         OW_MM_Y - OW_MM_PAD,
                         OW_MM_W + OW_MM_PAD * 2,
                         OW_MM_H + OW_MM_PAD * 2 + 14);

        iSetColor(180, 180, 180);
        iRectangle(OW_MM_X - OW_MM_PAD,
                   OW_MM_Y - OW_MM_PAD,
                   OW_MM_W + OW_MM_PAD * 2,
                   OW_MM_H + OW_MM_PAD * 2 + 14);

        iSetColor(200, 200, 200);
        iText(OW_MM_X, OW_MM_Y + OW_MM_H + 2, "MAP", (void*)0x0004);

        double scaleX = (double)OW_MM_W / OW_WORLD_WIDTH;
        double scaleY = (double)OW_MM_H / OW_WORLD_HEIGHT;
        double tileMMW = ceil(OW_MM_W / 5.0) + 1;
        double tileMMH = ceil(OW_MM_H / 3.0) + 1;

        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 5; c++) {
                if (!bgTiles[r][c]) continue;
                double wX = c * (OW_WORLD_WIDTH / 5.0);
                double wY = OW_WORLD_HEIGHT - (r + 1) * (OW_WORLD_HEIGHT / 3.0);
                int mmX = OW_MM_X + (int)(wX * scaleX);
                int mmY = OW_MM_Y + (int)(wY * scaleY);
                iShowImage(mmX, mmY, (int)tileMMW, (int)tileMMH, bgTiles[r][c]);
            }
        }

        int dotX = OW_MM_X + (int)(player.x * scaleX);
        int dotY = OW_MM_Y + (int)(player.y * scaleY);

        iSetColor(255, 255, 255);
        iFilledCircle(dotX, dotY, 4);
        iSetColor(255, 80, 80);
        iFilledCircle(dotX, dotY, 3);

        double halfW = (OW_SCREEN_WIDTH  / OW_ZOOM) / 2.0;
        double halfH = (OW_SCREEN_HEIGHT / OW_ZOOM) / 2.0;
        int vpX = OW_MM_X + (int)((camX) * scaleX);
        int vpY = OW_MM_Y + (int)((camY) * scaleY);
        int vpW = (int)(halfW * 2 * scaleX);
        int vpH = (int)(halfH * 2 * scaleY);
        iSetColor(255, 255, 100);
        iRectangle(vpX, vpY, vpW, vpH);
    }

private:
    OpenWorldGame()
        : initialized(false),
          dir(0), animFrame(0), isMoving(false), triggerCooldown(0.0),
          camX(0), camY(0), taskIcon(0), taskInterfaceImage(0), showTask1(false) {
        player.x = 810.0;
        player.y = 115.0;
        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 5; c++) {
                bgTiles[r][c] = 0;
                maskTiles[r][c] = NULL;
                maskW_tile[r][c] = 0;
                maskH_tile[r][c] = 0;
            }
        }
        for (int i = 0; i < 4; i++) taskPhaseImages[i] = 0;
    }
    
    OpenWorldGame(const OpenWorldGame&);
    void operator=(const OpenWorldGame&);

    bool             initialized;
    OWPlayer         player;
    double           camX, camY;
    double           triggerCooldown;
    
    unsigned int     bgTiles[3][5];
    unsigned char*   maskTiles[3][5];
    int              maskW_tile[3][5];
    int              maskH_tile[3][5];
    
    unsigned int     playerFrames[4][OW_NUM_FRAMES];
    unsigned int     taskIcon;
    unsigned int     taskInterfaceImage; // kept for fallback
    unsigned int     taskPhaseImages[4]; // phase 0,1,2,3
    bool             showTask1;
    int              dir;
    int              animFrame;
    bool             isMoving;
};

#endif // OPENWORLD_H
