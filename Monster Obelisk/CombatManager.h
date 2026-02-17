#ifndef COMBATMANAGER_H
#define COMBATMANAGER_H

#include <string>
#include <vector>
#include <cstdio>
#include <algorithm>
#include <cmath>

// Forward declarations if not already included
// This ensures Intellisense works, and the compiler is happy even if include order changes slightly (though iGraphics must come first for the libs)
extern void iShowBMP(int x, int y, const char* filename);
extern void iShowImage(int x, int y, int width, int height, unsigned int texture);
extern void iSetColor(double r, double g, double b);
extern void iFilledRectangle(double left, double bottom, double dx, double dy);
extern void iRectangle(double left, double bottom, double dx, double dy);
extern void iText(double x, double y, const char* str, void* font);
extern unsigned int iLoadImage(const char* filename, int rIgnore, int gIgnore, int bIgnore);
extern unsigned int iLoadImage(const char* filename);

// Enum must be visible
enum CombatState {
    COMBAT_START,
    PLAYER_TURN,
    ENEMY_TURN,
    PLAYER_ATTACK,
    ENEMY_ATTACK,
    WAVE_CLEAR,
    VICTORY,
    DEFEAT
};

struct Monster {
    std::string name;
    std::string textureIdle;
    std::string textureAttack;
    
    // Stats
    float maxHealth;
    float currentHealth;
    float attackPower;
    
    // Visual
    float x, y;
    unsigned int textureID; // Static/Fallback
    
    // Animations
    std::vector<unsigned int> walkFrames;
    std::vector<unsigned int> attack1Frames; // Lightning Blast
    std::vector<unsigned int> attack2Frames; // Thunder Crash
    
    int currentFrame;
    float animTimer;
    int currentAttackType; // 0=None, 1=LB, 2=TC

    Monster() : maxHealth(100), currentHealth(100), attackPower(10), x(0), y(0), textureID(0), currentFrame(0), animTimer(0), currentAttackType(0) {}
};

class CombatManager {
public:
    int lives;
    float bonusStrength;

    static CombatManager& GetInstance() {
        static CombatManager instance;
        return instance;
    }

    void InitCombat() {
        currentWave = 1;
        currentState = COMBAT_START;
        
        // Init Player
        playerMonster.name = "Vivi";
        playerMonster.textureIdle = "Image//lightning_monster_idle.bmp"; // Fallback
        playerMonster.textureID = iLoadImage(playerMonster.textureIdle.c_str(), 255, 255, 255); 
        
        // Load Animations for Vivi
        LoadAnimationFrames(playerMonster.walkFrames, "Image//Vivi//vivi_walk", 25);
        LoadAnimationFrames(playerMonster.attack1Frames, "Image//Vivi//vivi_lb", 25);
        LoadAnimationFrames(playerMonster.attack2Frames, "Image//Vivi//vivi_tc", 25);
        
        playerMonster.x = PLAYER_START_X;
        playerMonster.y = PLAYER_START_Y;
        
        backgroundTextureID = iLoadImage("Image//combact_background.bmp");
        SetupWave(currentWave);
    }
    
    void LoadAnimationFrames(std::vector<unsigned int>& frames, std::string prefix, int count) {
        frames.clear();
        char path[256];
        for (int i = 1; i <= count; i++) {
            sprintf_s(path, 256, "%s (%d).png", prefix.c_str(), i);
            frames.push_back(iLoadImage(path, 255, 255, 255)); // Removed white BG just in case
        }
    }

    void SetupWave(int waveIndex) {
        // Apply strength bonus
        playerMonster.attackPower = 40.0f + bonusStrength + (20.0f * (waveIndex - 1));
        playerMonster.maxHealth = 100.0f + (50.0f * (waveIndex - 1));
        playerMonster.currentHealth = playerMonster.maxHealth;

        enemyMonster.name = "Evil Minion";
        enemyMonster.textureIdle = "Image//evil_monster_idle.bmp";
        enemyMonster.textureID = iLoadImage(enemyMonster.textureIdle.c_str(), 255, 255, 255); // Remove white BG
        
        // Scale difficulty - Balanced
        enemyMonster.maxHealth = 250.0f * waveIndex;
        enemyMonster.currentHealth = enemyMonster.maxHealth;
        enemyMonster.attackPower = 10.0f + (10.0f * waveIndex); // 20, 30, 40...
        enemyMonster.x = ENEMY_START_X;
        enemyMonster.y = ENEMY_START_Y;
        
        currentState = PLAYER_TURN;
        waveMessageTimer = 3.0f; // Show wave message for 3 seconds
    }

    void MovePlayer(float dx, float dy) {
        // Allow movement during idle turns AND attack states
        // This fixes the "stuck after space bar" issue
        bool canMove = (currentState == PLAYER_TURN || 
                        currentState == ENEMY_TURN || 
                        currentState == COMBAT_START || 
                        currentState == PLAYER_ATTACK);
                        
        if (!canMove) return;

        playerMonster.x += dx;
        // playerMonster.y += dy; // REMOVED: Only X movement allowed
        
        // Simple Bounds Check
        if (playerMonster.x < 0) playerMonster.x = 0;
        if (playerMonster.x > 800) playerMonster.x = 800; // Expanded to full width if needed, or 400 limit? keeping 800
    }

    void PlayerAttack(int type) {
        if (currentState == PLAYER_TURN) {
            playerMonster.currentAttackType = type; // 1 or 2
            playerMonster.currentFrame = 0; // Reset animation
            playerMonster.animTimer = 0;
            
            if (type == 1) { // Space: Lightning Blast
                 currentAttackName = "LIGHTNING BLAST!";
                 currentAttackDamage = playerMonster.attackPower; 
            } else if (type == 2) { // F: Thunder Crash
                 if (currentWave < 3) return; 
                 currentAttackName = "THUNDER CRASH!";
                 currentAttackDamage = playerMonster.attackPower * 2.0f; 
            }
            
            currentState = PLAYER_ATTACK;
            stateTimer = 0;
            attackFeedbackTimer = 1.0f; // Show text for 1 second
        }
    }

    void UpdateCombat(float deltaTime) {
        if (waveMessageTimer > 0) waveMessageTimer -= deltaTime;
        if (attackFeedbackTimer > 0) attackFeedbackTimer -= deltaTime;
        stateTimer += deltaTime;

        // --- ANIMATION UPDATE ---
        playerMonster.animTimer += deltaTime;
        float frameDuration = 0.04f; // 25fps -> 1 second for 25 frames
        
        if (currentState == PLAYER_ATTACK) {
             // Attack Animation (Play Once)
             if (playerMonster.animTimer >= frameDuration) {
                 playerMonster.animTimer = 0;
                 playerMonster.currentFrame++;
                 
                 // Type 1: LB
                 if (playerMonster.currentAttackType == 1) {
                     if (playerMonster.currentFrame >= playerMonster.attack1Frames.size()) {
                         playerMonster.currentFrame = playerMonster.attack1Frames.size() - 1; // Hold last frame?
                     }
                 }
                 // Type 2: TC
                 else if (playerMonster.currentAttackType == 2) {
                     if (playerMonster.currentFrame >= playerMonster.attack2Frames.size()) {
                         playerMonster.currentFrame = playerMonster.attack2Frames.size() - 1;
                     }
                 }
             }
        } else {
             // Walk/Idle Animation (Loop)
             if (playerMonster.animTimer >= frameDuration) {
                 playerMonster.animTimer = 0;
                 playerMonster.currentFrame++;
                 if (playerMonster.currentFrame >= playerMonster.walkFrames.size()) {
                     playerMonster.currentFrame = 0;
                 }
             }
        }
        // ------------------------

        // WAVE CLEAR LOGIC
        if (currentState == WAVE_CLEAR) {
             if (stateTimer > 2.0f) { // Wait 2 seconds before next wave
                 SetupWave(currentWave);
             }
             return; // Don't process other logic
        }

        // Real-time Enemy Attacks
        if (currentState != WAVE_CLEAR && currentState != VICTORY && currentState != DEFEAT) {
             enemyAttackTimer -= deltaTime;
             float dx = playerMonster.x - enemyMonster.x;
             float dist = fabs(dx);

             // If in range and cooldown ready, ATTACK!
             if (dist < 150.0f && enemyAttackTimer <= 0) {
                 playerMonster.currentHealth -= enemyMonster.attackPower;
                 enemyAttackTimer = 1.0f; // Limit attack speed (1 attack per second)
                 
                 if (playerMonster.currentHealth <= 0) {
                     playerMonster.currentHealth = 0;
                     currentState = DEFEAT;
                 }
             }
        }

        // Enemy AI: Move towards player to fight (X tracking only)
        // Only move during "idle" turns or when waiting to attack
        if (currentState == PLAYER_TURN || currentState == ENEMY_TURN) {
            float dx = playerMonster.x - enemyMonster.x;
            float distance = fabs(dx); // Only X distance matters now
            const float stopDistance = 150.0f; // Attack range
            
            if (distance > stopDistance) {
                float speed = 100.0f; // Pixels per second
                
                // Move towards player
                if (dx > 0) enemyMonster.x += speed * deltaTime;
                else enemyMonster.x -= speed * deltaTime;
            } else {
                // If close enough and in enemy turn, attack immediately!
                if (currentState == ENEMY_TURN && stateTimer > 0.5f) { // Added small delay so it doesn't spam instantly
                    currentState = ENEMY_ATTACK;
                    stateTimer = 0;
                }
            }
            
            // Keep bounds (Y is fixed now)
             if (enemyMonster.x < 0) enemyMonster.x = 0;
            if (enemyMonster.x > 800) enemyMonster.x = 800;
        }

        switch (currentState) {
        case PLAYER_TURN:
            break;
            
        case PLAYER_ATTACK:
            if (stateTimer > 0.25f) { // Very fast attack
                // Check distance for hit
                float dx = playerMonster.x - enemyMonster.x;
                float dy = playerMonster.y - enemyMonster.y;
                float dist = sqrt(dx*dx + dy*dy);
                
                if (dist < 250.0f) { // Range 250 (monsters are 200px wide, so they must be touching/close)
                    float damage = currentAttackDamage; 
                    enemyMonster.currentHealth -= damage;
                    playerHit = true; // Visual flag
                    
                    if (enemyMonster.currentHealth <= 0) {
                        enemyMonster.currentHealth = 0;
                        EndWave();
                        return; // Stop processing
                    }
                } else {
                     playerHit = false; // Miss
                }

                currentState = PLAYER_TURN; // Back to player control immediately (Real-time)
                stateTimer = 0;
            }
            break;
            
        // Turn-based states removed/ignored for real-time flow
        case ENEMY_TURN:
        case ENEMY_ATTACK:
            currentState = PLAYER_TURN; // Just in case, force to player turn
            break;
        }
    }

    void RenderCombat() {
        // Draw Background
        // Draw Background
        iShowImage(0, 0, 1000, 600, backgroundTextureID);
        
        // Draw Player (Animated)
        unsigned int tex = playerMonster.textureID;
        
        if (currentState == PLAYER_ATTACK) {
             if (playerMonster.currentAttackType == 1 && !playerMonster.attack1Frames.empty()) {
                 tex = playerMonster.attack1Frames[playerMonster.currentFrame % playerMonster.attack1Frames.size()];
             }
             else if (playerMonster.currentAttackType == 2 && !playerMonster.attack2Frames.empty()) {
                 tex = playerMonster.attack2Frames[playerMonster.currentFrame % playerMonster.attack2Frames.size()];
             }
        } else {
             // Idle/Walk
             if (!playerMonster.walkFrames.empty()) {
                 tex = playerMonster.walkFrames[playerMonster.currentFrame % playerMonster.walkFrames.size()];
             }
        }

        iShowImage((int)playerMonster.x, (int)playerMonster.y, 200, 200, tex);
        DrawHealthBar(playerMonster.x + 20, playerMonster.y + 210, playerMonster.currentHealth, playerMonster.maxHealth, 0, 255, 0);  
        // HP Text
        char hpBuffer[32];
        sprintf_s(hpBuffer, 32, "HP: %d/%d", (int)playerMonster.currentHealth, (int)playerMonster.maxHealth);
        iSetColor(255, 255, 255);
        iText(playerMonster.x + 20, playerMonster.y + 225, hpBuffer, (void*)0x0008);

        // Draw Enemy (Scaled to 200x200)
        iShowImage((int)enemyMonster.x, (int)enemyMonster.y, 200, 200, enemyMonster.textureID);
        DrawHealthBar(enemyMonster.x + 20, enemyMonster.y + 210, enemyMonster.currentHealth, enemyMonster.maxHealth, 255, 0, 0);
        // HP Text
        sprintf_s(hpBuffer, 32, "HP: %d/%d", (int)enemyMonster.currentHealth, (int)enemyMonster.maxHealth);
        iSetColor(255, 255, 255);
        iText(enemyMonster.x + 20, enemyMonster.y + 225, hpBuffer, (void*)0x0008);
        
    // Attack Feedback
        if (attackFeedbackTimer > 0) {
             iSetColor(255, 0, 0);
             iText(playerMonster.x, playerMonster.y + 300, currentAttackName.c_str(), (void*)0x0008);
        }

        // Wave Notification and Clear
        if (currentState == WAVE_CLEAR) {
             iSetColor(0, 255, 0);
             iText(400, 500, "WAVE CLEARED!", (void*)0x0005);
        } else if (waveMessageTimer > 0) {
             iSetColor(255, 215, 0);
             char waveMsg[64];
             if (currentWave == 3) {
                 sprintf_s(waveMsg, 64, "WAVE 3: THUNDER CRASH UNLOCKED!");
             } else {
                 sprintf_s(waveMsg, 64, "WAVE %d STARTED!", currentWave);
             }
             iText(300, 450, waveMsg, (void*)0x0005); 
        }
        
        // UI - Button Removed
        // Status
        
        // Status
        if (currentState == VICTORY) {
            iSetColor(255, 215, 0);
            iText(400, 500, "VICTORY!", (void*)0x0006); // TIMES_ROMAN_24
        } else if (currentState == DEFEAT) {
            iSetColor(255, 0, 0);
            iText(400, 500, "DEFEAT...", (void*)0x0006);
            iSetColor(255, 255, 255);
            iText(350, 450, "Click to Try Again", (void*)0x0005);
        } else {
            char statusBuf[64];
            sprintf_s(statusBuf, 64, "Wave: %d / %d", currentWave, TOTAL_WAVES);
            iSetColor(255, 255, 255);
            iText(400, 550, statusBuf, (void*)0x0008);
        }

        // Draw Lives
        iSetColor(255, 255, 0);
        char lifeBuf[32];
        sprintf_s(lifeBuf, 32, "LIVES: %d", lives);
        iText(50, 550, lifeBuf, (void*)0x0008);
    }

    void OnCleanClick(int mx, int my) {
        // Attack Button Removed
        // if (mx >= 400 && mx <= 600 && my >= 50 && my <= 100) { ... }
        
        // Try Again on Defeat
        if (currentState == DEFEAT) {
            RetryWave();
        }
    }

    void RetryWave() {
        lives--;
        if (lives <= 0) {
            extern int gameState;
            gameState = 1; // Kick back to map
            // lives = 3; // REMOVED: Do not reset lives automatically
            return;
        }

        playerMonster.currentHealth = playerMonster.maxHealth; 
        SetupWave(currentWave);
    }

    void RestoreLives() {
        lives = 4;
    }

    void AddStrengthBonus(float bonus) {
        bonusStrength += bonus;
    }

private:
    CombatManager() : 
        lives(3),
        bonusStrength(0),
        TOTAL_WAVES(3),
        PLAYER_START_X(100), 
        PLAYER_START_Y(50), 
        ENEMY_START_X(700),
        ENEMY_START_Y(50),
        backgroundTextureID(0),
        stateTimer(0),
        waveMessageTimer(0),
        enemyAttackTimer(0),
        attackFeedbackTimer(0),
        playerHit(false)
    {
        // InitCombat(); // REMOVED: Must be called manually after OpenGL context is created!
    }
    
    // Legacy non-copyable
    CombatManager(const CombatManager&);
    void operator=(const CombatManager&);

    void EndWave() {
        if (currentWave < TOTAL_WAVES) {
            currentWave++;
            
            // Increase stats
            playerMonster.maxHealth += 50.0f; 
            playerMonster.currentHealth = playerMonster.maxHealth; 
            playerMonster.attackPower += 20.0f; // Consistent growth 

            // Only switch state to WAVE_CLEAR, do NOT SetupWave yet
            currentState = WAVE_CLEAR;
            stateTimer = 0; 
        } else {
            currentState = VICTORY;
        }
    }

    void DrawHealthBar(float x, float y, float current, float max, float r, float g, float b) {
        iSetColor(100, 100, 100);
        iFilledRectangle(x, y, 100, 10);
        
        if (max > 0) {
            float ratio = current / max;
            if (ratio < 0) ratio = 0;
            iSetColor(r, g, b);
            iFilledRectangle(x, y, 100 * ratio, 10);
        }
        
        iSetColor(255, 255, 255);
        iRectangle(x, y, 100, 10);
    }

    Monster playerMonster;
    Monster enemyMonster;
    CombatState currentState;
    int currentWave;
    float stateTimer;
    float waveMessageTimer;
    float enemyAttackTimer; // Independent attack cooldown
    float attackFeedbackTimer; 
    std::string currentAttackName;
    float currentAttackDamage;
    bool playerHit;
    unsigned int backgroundTextureID; // Cache background
    
    const int TOTAL_WAVES;
    const float PLAYER_START_X;
    const float PLAYER_START_Y;
    const float ENEMY_START_X;
    const float ENEMY_START_Y;
};

#endif
