#ifndef RUNNERGAME_H
#define RUNNERGAME_H

#include <vector>
#include <string>
#include <cmath>
#include <ctime>
#include <cstdlib>

// Forward declarations for iGraphics functions
extern void iShowImage(int x, int y, int width, int height, unsigned int texture);
extern void iSetColor(double r, double g, double b);
extern void iText(double x, double y, const char* str, void* font);
extern void iFilledRectangle(double left, double bottom, double dx, double dy);
extern unsigned int iLoadImage(const char* filename, int rIgnore, int gIgnore, int bIgnore);
extern unsigned int iLoadImage(const char* filename);

// Entity Structure
struct RunnerEntity {
    float x, y;
    int width, height;
    unsigned int textureID;
    bool active;
    int type; // 0=Stone, 1=Monster, 2=Crystal
    
    RunnerEntity(float _x, float _y, int _w, int _h, unsigned int _tex, int _type) 
        : x(_x), y(_y), width(_w), height(_h), textureID(_tex), active(true), type(_type) {}
};

class RunnerGame {
public:
    static RunnerGame& GetInstance() {
        static RunnerGame instance;
        return instance;
    }

    void Init() {
        if (initialized) return;

        // Load Kael Animations
        LoadAnimationFrames(kaelRunTextures, "Image//Kael//Kael_running", 16);
        LoadAnimationFrames(kaelJumpTextures, "Image//Kael//Kael_jumping", 16);
        
        // Load Vivi Animations (Unlockable)
        LoadAnimationFrames(viviRunTextures, "Image//Vivi//vivi_walk", 25);
        
        // Load Environment
        bgLayers[0] = iLoadImage("Image//runbg1.png");
        bgLayers[1] = iLoadImage("Image//runbg2.png");
        
        // obstacles
        lavaStoneTex = iLoadImage("Image//lava stone.png"); // Use native transparency
        monsterTex = iLoadImage("Image//black_running (1).png", 0, 0, 0); // Black bg removal if BMP? Or wait, usually white or specific color. 
        // Based on other code, 255,255,255 might be transparent color. Let's try 255,255,255.
        // Re-loading monster with transparency if needed. 
        // Actually iLoadImage without args doesn't key out color. 
        // Let's assume transparent PNGs for lava stone.
        // Monster is BMP. Code uses 255,255,255 in CombatManager. 
        monsterTex = iLoadImage("Image//black_running (1).png", 255, 255, 255); 

        // Crystals
        crystalTex = iLoadImage("Image//purple crystal.png", 255, 255, 255);

        Reset();
        initialized = true;
    }

    void LoadAnimationFrames(std::vector<unsigned int>& frames, std::string prefix, int count) {
        frames.clear();
        char path[256];
        for (int i = 1; i <= count; i++) {
            sprintf_s(path, 256, "%s (%d).png", prefix.c_str(), i);
            frames.push_back(iLoadImage(path));
        }
    }

    void UnloadResources() {
        // Optional: clear textures if needed
    }

    void Reset() {
        playerX = 100;
        playerY = GROUND_Y;
        velocityY = 0;
        isJumping = false;
        isGameOver = false;
        showUnlockMessage = false; // Reset message flag
        score = 0;
        coins = 0;
        entities.clear();
        spawnTimer = 0;
        
        bgX1 = 0;
        bgX2 = 1000; // Screen width
        
        currentFrame = 0;
        animTimer = 0;
        
        moveSpeed = 400.0f; // Start speed
    }

    void Update(float deltaTime) {
        if (isGameOver || showUnlockMessage) return;

        UpdatePhysics(deltaTime);
        UpdateBackground(deltaTime);
        UpdateEntities(deltaTime);
        UpdateAnimation(deltaTime);

        // Increase difficulty
        moveSpeed += 5 * deltaTime; 
    }

    void Render() {
        DrawBackground();
        DrawPlayer();
        DrawEntities();
        DrawUI();
    }

    void HandleInput(unsigned char key) {
        if (showUnlockMessage) {
            if (key == ' ') {
                showUnlockMessage = false;
                extern int gameState; 
                gameState = 1; // Return to Map
                Reset(); 
            }
            return;
        }

        if (isGameOver) {
            if (key == ' ') Reset();
            return;
        }

        if (key == ' ' && !isJumping) {
            isJumping = true;
            velocityY = JUMP_FORCE;
        }
    }

private:
    void UpdatePhysics(float deltaTime) {
        if (isJumping) {
            playerY += velocityY * deltaTime;
            velocityY -= GRAVITY * deltaTime;
            if (playerY <= GROUND_Y) {
                playerY = GROUND_Y;
                isJumping = false;
                velocityY = 0;
            }
        }
    }

    void UpdateBackground(float deltaTime) {
        float bgSpeed = moveSpeed;
        bgX1 -= bgSpeed * deltaTime;
        bgX2 -= bgSpeed * deltaTime;
        if (bgX1 <= -1000) bgX1 = bgX2 + 1000 - 5;
        if (bgX2 <= -1000) bgX2 = bgX1 + 1000 - 5;
    }

    void UpdateEntities(float deltaTime) {
        spawnTimer -= deltaTime;
        if (spawnTimer <= 0) {
            SpawnEntity();
            spawnTimer = 1.5f + (rand() % 100) / 100.0f;
        }

        for (int i = 0; i < (int)entities.size(); i++) {
            if (!entities[i].active) continue;
            entities[i].x -= moveSpeed * deltaTime;
            
            if (entities[i].x + entities[i].width < 0) {
                entities[i].active = false;
            } else {
                CheckCollision(entities[i]);
            }
        }
    }

    void CheckCollision(RunnerEntity& e) {
        // Simple AABB Collision
        if (playerX + 30 < e.x + 10 + e.width - 20 &&
            playerX + 30 + 40 > e.x + 10 &&
            playerY < e.y + 10 + e.height - 20 &&
            playerY + 80 > e.y + 10) {
            
            if (e.type == 2) { // Crystal
                coins++;
                e.active = false;
                if (coins >= 5 && !viviUnlocked) {
                    viviUnlocked = true;
                    showUnlockMessage = true;
                }
            } else { // Obstacle
                isGameOver = true;
            }
        }
    }

    void UpdateAnimation(float deltaTime) {
        animTimer += deltaTime;
        if (animTimer >= 0.05f) {
            animTimer = 0;
            currentFrame++;
        }
    }

    void DrawBackground() {
        iShowImage((int)bgX1, 0, 1000, 600, bgLayers[0]);
        iShowImage((int)bgX2, 0, 1000, 600, bgLayers[1]);
    }

    void DrawPlayer() {
        std::vector<unsigned int>* currentAnim;
        if (currentCharacter == 0) { // Kael
            currentAnim = isJumping ? &kaelJumpTextures : &kaelRunTextures;
        } else { // Vivi
            currentAnim = &viviRunTextures;
        }
        
        if (!currentAnim->empty()) {
            unsigned int tex = (*currentAnim)[currentFrame % currentAnim->size()];
            iShowImage((int)playerX, (int)playerY, 100, 100, tex);
        }
    }

    void DrawEntities() {
        for (int i = 0; i < (int)entities.size(); i++) {
            if (entities[i].active) {
                iShowImage((int)entities[i].x, (int)entities[i].y, entities[i].width, entities[i].height, entities[i].textureID);
            }
        }
    }

    void DrawUI() {
        char msg[128];
        sprintf_s(msg, 128, "Crystals: %d", coins);
        iSetColor(255, 215, 0);
        iText(800, 550, msg, (void*)0x0008);
        
        if (showUnlockMessage) {
            iSetColor(0, 0, 0);
            iFilledRectangle(200, 200, 600, 200);
            iSetColor(0, 255, 0);
            iText(350, 320, "New Monster Unlocked!", (void*)0x0006);
            iSetColor(255, 255, 255);
            iText(330, 280, "Press SPACE to return to Map", (void*)0x0008);
        } else if (isGameOver) {
            iSetColor(0, 0, 0);
            iFilledRectangle(300, 250, 400, 100);
            iSetColor(255, 0, 0);
            iText(420, 300, "GAME OVER", (void*)0x0006);
            iSetColor(255, 255, 255);
            iText(380, 270, "Press SPACE to Restart", (void*)0x0008);
        }
    }

private:
    RunnerGame() : 
        initialized(false), 
        currentCharacter(0), 
        viviUnlocked(false),
        showUnlockMessage(false),
        GROUND_Y(50.0f),
        GRAVITY(2000.0f),
        JUMP_FORCE(900.0f),
        // Initialize other primitive members
        playerX(100.0f),
        playerY(50.0f),
        velocityY(0.0f),
        isJumping(false),
        moveSpeed(400.0f),
        bgX1(0.0f),
        bgX2(1000.0f),
        score(0),
        coins(0),
        isGameOver(false),
        currentFrame(0),
        animTimer(0.0f),
        spawnTimer(0.0f),
        // Texture placeholders
        lavaStoneTex(0),
        monsterTex(0),
        crystalTex(0)
    {
         bgLayers[0] = 0;
         bgLayers[1] = 0;
    }
    
    // Copy control
    RunnerGame(const RunnerGame&);
    void operator=(const RunnerGame&);

    bool initialized;
    
    // Resources
    std::vector<unsigned int> kaelRunTextures;
    std::vector<unsigned int> kaelJumpTextures;
    std::vector<unsigned int> viviRunTextures;
    unsigned int bgLayers[2];
    unsigned int lavaStoneTex;
    unsigned int monsterTex;
    unsigned int crystalTex;

    // Game Variables
    const float GROUND_Y;
    const float GRAVITY;
    const float JUMP_FORCE;
    
    float playerX, playerY;
    float velocityY;
    bool isJumping;
    
    int currentCharacter; // 0=Kael, 1=Vivi
    bool viviUnlocked;
    bool showUnlockMessage; // New flag
    
    float moveSpeed;
    float bgX1, bgX2;
    
    int score;
    int coins;
    bool isGameOver;
    
    // Animation
    int currentFrame;
    float animTimer;
    
    // Spawning
    std::vector<RunnerEntity> entities;
    float spawnTimer;
};

#endif
