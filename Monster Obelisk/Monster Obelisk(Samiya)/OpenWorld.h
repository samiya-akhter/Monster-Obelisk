#ifndef OPENWORLD_H
#define OPENWORLD_H

#include "iGraphics.h"
#include <math.h>
#include <stdio.h>
#include <windows.h>
#include "CombatManager.h"

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
#define OW_ZOOM          1.5
#define OW_NUM_FRAMES    9

struct OWPlayer { double x, y; };

class OpenWorldGame {
public:
    static OpenWorldGame& GetInstance() {
        static OpenWorldGame inst;
        return inst;
    }

    // ---------------------------------------------------------------
    // Load a large image using mipmaps (supports textures > 1024 px)
    // ---------------------------------------------------------------
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

    // ---------------------------------------------------------------
    // Called once when the right-side tower is clicked
    // ---------------------------------------------------------------
    void Init() {
        // Always reset player start position
        player.x = 810.0;
        player.y = 115.0;

        if (initialized) {
            printf("[OpenWorld] Already initialized, just reset position.\n");
            return;
        }

        printf("[OpenWorld] Initializing...\n");

        // Load background map
        backgroundImg = LoadHugeImage("Image\\open world map.png");

        // Load player sprite frames
        char path[128];
        const char* dirs[4] = { "down", "up", "left", "right" };
        for (int d = 0; d < 4; d++) {
            for (int f = 0; f < OW_NUM_FRAMES; f++) {
                int fn = (d == 3 && f == 8) ? 9 : f;
                sprintf_s(path, sizeof(path), "Image\\player-%s-%d.png", dirs[d], fn);
                playerFrames[d][f] = iLoadImage(path);
            }
        }

        // Load mask map — try multiple path variants since the working
        // directory differs between Debug run and project-dir run
        const char* maskPaths[] = {
            "Image\\mask map.png",
            "Image/mask map.png",
            "..\\Monster Obelisk\\Image\\mask map.png",
            "../Monster Obelisk/Image/mask map.png"
        };
        int ch = 0;
        maskData = NULL;
        for (int i = 0; i < 4; i++) {
            maskData = stbi_load(maskPaths[i], &maskW, &maskH, &ch, 3);
            if (maskData) {
                printf("[OpenWorld] Mask map loaded from: %s (%dx%d)\n", maskPaths[i], maskW, maskH);
                break;
            } else {
                printf("[OpenWorld] Tried path: %s -- not found\n", maskPaths[i]);
            }
        }
        if (!maskData) {
            printf("[OpenWorld] ERROR: Could not load mask map from any path!\n");
        } else {
            // Show color at player start to confirm it's reading correctly
            int mx = (int)(player.x * maskW / (double)OW_WORLD_WIDTH);
            int my = (int)((OW_WORLD_HEIGHT - player.y) * maskH / (double)OW_WORLD_HEIGHT);
            if (mx >= 0 && mx < maskW && my >= 0 && my < maskH) {
                int idx = (my * maskW + mx) * 3;
                printf("[OpenWorld] Mask color at start pos (%.0f,%.0f): R=%d G=%d B=%d -> %s\n",
                    player.x, player.y,
                    maskData[idx], maskData[idx+1], maskData[idx+2],
                    (maskData[idx]<60 && maskData[idx+1]<60 && maskData[idx+2]<60) ? "ON PATH (black)" : "OFF PATH (wall)");
            }
        }

        initialized = true;
    }

    // Set a short cooldown so they don't immediately re-trigger and can walk away
    void SetTriggerCooldown(double seconds) {
        triggerCooldown = seconds;
    }

    // ---------------------------------------------------------------
    // Called by the animation timer
    // ---------------------------------------------------------------
    void UpdateAnimation() {
        if (isMoving) {
            animFrame = (animFrame + 1) % OW_NUM_FRAMES;
        } else {
            animFrame = 0;
        }
    }

    // ---------------------------------------------------------------
    // Returns true if player is in the battle tower 1 trigger zone (Red map area)
    // ---------------------------------------------------------------
    bool CheckForBattleTower1Trigger() {
        if (triggerCooldown > 0.0) return false;
        if (!maskData) return false;

        int mx = (int)(player.x * maskW / (double)OW_WORLD_WIDTH);
        int my = (int)((OW_WORLD_HEIGHT - player.y) * maskH / (double)OW_WORLD_HEIGHT);

        if (mx < 0 || mx >= maskW || my < 0 || my >= maskH) return false;

        int idx = (my * maskW + mx) * 3;
        unsigned char r = maskData[idx];
        unsigned char g = maskData[idx + 1];
        unsigned char b = maskData[idx + 2];

        // Red zone check
        return (r > 100 && g < 100 && b < 100);
    }

    // ---------------------------------------------------------------
    // Returns true if player is in the battle tower 2 trigger zone (Green map area)
    // ---------------------------------------------------------------
    bool CheckForBattleTower2Trigger() {
        if (triggerCooldown > 0.0) return false;
        if (!maskData) return false;

        int mx = (int)(player.x * maskW / (double)OW_WORLD_WIDTH);
        int my = (int)((OW_WORLD_HEIGHT - player.y) * maskH / (double)OW_WORLD_HEIGHT);

        if (mx < 0 || mx >= maskW || my < 0 || my >= maskH) return false;

        int idx = (my * maskW + mx) * 3;
        unsigned char r = maskData[idx];
        unsigned char g = maskData[idx + 1];
        unsigned char b = maskData[idx + 2];

        // Green zone check
        return (g > 100 && r < 100 && b < 100);
    }

    // ---------------------------------------------------------------
    // Returns true if player is in the wild-area trigger zone (Blue map area)
    // ---------------------------------------------------------------
    bool CheckForWildAreaTrigger() {
        if (triggerCooldown > 0.0) return false;
        if (!maskData) return false;

        int mx = (int)(player.x * maskW / (double)OW_WORLD_WIDTH);
        int my = (int)((OW_WORLD_HEIGHT - player.y) * maskH / (double)OW_WORLD_HEIGHT);

        if (mx < 0 || mx >= maskW || my < 0 || my >= maskH) return false;

        int idx = (my * maskW + mx) * 3;
        unsigned char r = maskData[idx];
        unsigned char g = maskData[idx + 1];
        unsigned char b = maskData[idx + 2];

        // Blue zone check
        return (b > 100 && r < 100 && g < 100);
    }

    // ---------------------------------------------------------------
    // Returns true if player is in the endgame houses trigger zone (top-left)
    // The wild area trigger is Blue; houses zone will use the same blue check
    // since they share the top-left village area. We detect proximity instead.
    // ---------------------------------------------------------------
    bool CheckForHousesTrigger() {
        // Top-left village area: world coords roughly x < 350, y > 750
        return (player.x < 350.0 && player.y > 750.0);
    }

    // ---------------------------------------------------------------
    // Returns true if player is in the battle tower 3 trigger zone (Yellow map area)
    // ---------------------------------------------------------------
    bool CheckForBattleTower3Trigger() {
        if (triggerCooldown > 0.0) return false;
        if (!maskData) return false;

        int mx = (int)(player.x * maskW / (double)OW_WORLD_WIDTH);
        int my = (int)((OW_WORLD_HEIGHT - player.y) * maskH / (double)OW_WORLD_HEIGHT);

        if (mx < 0 || mx >= maskW || my < 0 || my >= maskH) return false;

        int idx = (my * maskW + mx) * 3;
        unsigned char r = maskData[idx];
        unsigned char g = maskData[idx + 1];
        unsigned char b = maskData[idx + 2];

        // Yellow zone check
        return (r > 100 && g > 100 && b < 100);
    }

    // ---------------------------------------------------------------
    // Returns true when the world-space point (x,y) is NOT on a
    // black pixel — meaning it is a wall in this mask map convention.
    // Black (R<60, G<60, B<60) = walkable path.
    // Any other color = solid wall.
    // ---------------------------------------------------------------
    bool IsWall(double x, double y) {
        if (!maskData) {
            return false;   // No mask loaded — allow all movement
        }

        // Map world coords → mask pixel coords
        int mx = (int)(x  * maskW / (double)OW_WORLD_WIDTH);
        int my = (int)((OW_WORLD_HEIGHT - y) * maskH / (double)OW_WORLD_HEIGHT);

        // Out-of-bounds → treat as wall
        if (mx < 0 || mx >= maskW || my < 0 || my >= maskH) return true;

        int idx = (my * maskW + mx) * 3;
        unsigned char r = maskData[idx];
        unsigned char g = maskData[idx + 1];
        unsigned char b = maskData[idx + 2];

        // Walkable pixels:
        //   Black (R<60, G<60, B<60)            = normal path
        //   Green (G>100, R<100, B<100)         = battle tower 2 trigger zone — still walkable!
        //   Blue (B>100, R<100, G<100)          = wild area trigger zone — still walkable!
        //   Red (R>100, G<100, B<100)           = battle tower 1 trigger zone (residual) — still walkable!
        //   Yellow (R>100, G>100, B<100)        = battle tower 3 trigger zone — still walkable!
        // Everything else (white, grey, etc.)   = solid wall
        bool onPath       = (r < 60 && g < 60 && b < 60);
        bool onBattleTower2Zone = (g > 100 && r < 100 && b < 100);
        bool onWildAreaZone = (b > 100 && r < 100 && g < 100);
        bool onBattleTower1Zone = (r > 100 && g < 100 && b < 100);
        bool onBattleTower3Zone = (r > 100 && g > 100 && b < 100);

        return !(onPath || onBattleTower2Zone || onWildAreaZone || onBattleTower1Zone || onBattleTower3Zone); // wall only if NOT walkable
    }

    // ---------------------------------------------------------------
    // Main update: reads input, moves player with collision
    // ---------------------------------------------------------------
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

        // Try X movement
        double nx = player.x + dx;
        double ny = player.y + dy;

        if (!IsWall(nx, player.y)) player.x = nx;
        if (!IsWall(player.x, ny)) player.y = ny;

        // Hard boundaries
        if (player.x < 0)               player.x = 0;
        if (player.y < 0)               player.y = 0;
        if (player.x > OW_WORLD_WIDTH)  player.x = OW_WORLD_WIDTH;
        if (player.y > OW_WORLD_HEIGHT) player.y = OW_WORLD_HEIGHT;

        // Console output while moving
        if (isMoving) {
            // Sample mask color for debugging
            int mx = (int)(player.x * maskW / (double)OW_WORLD_WIDTH);
            int my = (int)((OW_WORLD_HEIGHT - player.y) * maskH / (double)OW_WORLD_HEIGHT);
            if (maskData && mx >= 0 && mx < maskW && my >= 0 && my < maskH) {
                int idx = (my * maskW + mx) * 3;
                printf("Pos(%.1f,%.1f) MaskPixel R=%d G=%d B=%d %s\n",
                    player.x, player.y,
                    maskData[idx], maskData[idx+1], maskData[idx+2],
                    IsWall(player.x, player.y) ? "[WALL]" : "[PATH]");
            } else {
                printf("Pos(%.1f,%.1f) [OUT OF MASK BOUNDS]\n", player.x, player.y);
            }
        }

        // Update camera
        double halfW = (OW_SCREEN_WIDTH  / OW_ZOOM) / 2.0;
        double halfH = (OW_SCREEN_HEIGHT / OW_ZOOM) / 2.0;
        camX = player.x - halfW;
        camY = player.y - halfH;
        if (camX < 0) camX = 0;
        if (camY < 0) camY = 0;
        if (camX > OW_WORLD_WIDTH  - halfW * 2) camX = OW_WORLD_WIDTH  - halfW * 2;
        if (camY > OW_WORLD_HEIGHT - halfH * 2) camY = OW_WORLD_HEIGHT - halfH * 2;
    }

    // ---------------------------------------------------------------
    // Render
    // ---------------------------------------------------------------
    void Render() {
        if (!backgroundImg) return;

        // Draw world map
        iShowImage(
            (int)((0    - camX) * OW_ZOOM),
            (int)((0    - camY) * OW_ZOOM),
            (int)(OW_WORLD_WIDTH  * OW_ZOOM),
            (int)(OW_WORLD_HEIGHT * OW_ZOOM),
            backgroundImg);

        // Draw player sprite
        double sz = 32.0 * OW_ZOOM;
        iShowImage(
            (int)((player.x - camX) * OW_ZOOM - sz / 2.0),
            (int)((player.y - camY) * OW_ZOOM - sz / 2.0),
            (int)sz, (int)sz,
            playerFrames[dir][animFrame]);

        // Prompt text for Tower 3
        if (CheckForBattleTower3Trigger()) {
            iSetColor(255, 255, 0);
            iText(300, 550, "Press 'Enter' to enter Tower - 3", (void*)0x0005);
        }

        // Prompt text for Tower 2
        if (CheckForBattleTower2Trigger()) {
            iSetColor(0, 255, 0);
            iText(300, 550, "Press 'Enter' to enter Tower - 2", (void*)0x0005);
        }
        
        // Prompt text for Tower 1
        if (CheckForBattleTower1Trigger()) {
            iSetColor(255, 0, 0);
            iText(300, 550, "Press 'Enter' to enter Tower - 1", (void*)0x0005);
        }

        // Endgame hint: all towers cleared, player must go top-left
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
    }

private:
    OpenWorldGame()
        : initialized(false), backgroundImg(0),
          maskData(NULL), maskW(0), maskH(0),
          dir(0), animFrame(0), isMoving(false), triggerCooldown(0.0),
          camX(0), camY(0) {
        player.x = 810.0;
        player.y = 115.0;
    }
    OpenWorldGame(const OpenWorldGame&);
    void operator=(const OpenWorldGame&);

    bool             initialized;
    OWPlayer         player;
    double           camX, camY;
    double           triggerCooldown;
    unsigned int     backgroundImg;
    unsigned char*   maskData;
    int              maskW, maskH;
    unsigned int     playerFrames[4][OW_NUM_FRAMES];
    int              dir;
    int              animFrame;
    bool             isMoving;
};

#endif // OPENWORLD_H
