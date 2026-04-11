#ifndef FINALBOSSMANAGER_H
#define FINALBOSSMANAGER_H

#include <vector>
#include <string>

// Forward declarations bridging to iGraphics
extern void iShowImage(int x, int y, int width, int height, unsigned int texture);
extern void iSetColor(double r, double g, double b);
extern void iFilledRectangle(double left, double bottom, double dx, double dy);
extern void iRectangle(double left, double bottom, double dx, double dy);
extern void iText(double x, double y, const char* str, void* font);
extern unsigned int iLoadImage(const char* filename, int rIgnore, int gIgnore, int bIgnore);
extern unsigned int iLoadImage(const char* filename);

enum FinalBossState {
    FB_PLAYING,
    FB_VICTORY,
    FB_DEFEAT
};

class FinalBossManager {
private:
    float playerX, playerY;
    float playerHP, playerMaxHP;
    int playerFrame;
    int playerAttackFrame;
    float playerAnimTimer;
    bool playerFacingRight;
    bool playerIsMoving;
    int playerAttackType; // 0 = idle/sprint, 1 = lb, 2 = tc
    float playerAttackTimer;

    std::vector<unsigned int> playerSprintRight;
    std::vector<unsigned int> playerSprintLeft;
    std::vector<unsigned int> playerAttackRight;
    std::vector<unsigned int> playerAttackLeft;
    std::vector<unsigned int> playerTCAttackRight;
    std::vector<unsigned int> playerTCAttackLeft;
    std::vector<unsigned int> playerJumpRight;
    std::vector<unsigned int> playerJumpLeft;

    float velocityY;
    bool isJumping;
    float basePlayerY;
    const float GRAVITY = 1500.0f;
    const float JUMP_SPEED = 600.0f;

    float bossX, bossY;
    float bossHP, bossMaxHP;
    int bossFrame;
    float bossAnimTimer;
    bool bossFacingRight;
    int bossAttackType; // 0 = walk, 1 = normal attack, 2 = special attack
    
    std::vector<unsigned int> mogamboWalkL;
    std::vector<unsigned int> mogamboWalkR;
    std::vector<unsigned int> mogamboAtkL;
    std::vector<unsigned int> mogamboAtkR;
    std::vector<unsigned int> mogamboSpAtkL;
    std::vector<unsigned int> mogamboSpAtkR;
    std::vector<unsigned int> mogamboSmashL;
    std::vector<unsigned int> mogamboSmashR;

    int playerHitCount;
    float redGlowTimer;
    float specialAttackTimer;
    float groundSmashTimer;
    float normalAttackTimer;
    bool useGroundSmashNext;
    bool hasTakenSpecialDamage;

    unsigned int backgroundTex;
    FinalBossState currentState;

    void LoadFramesZeroIndexed(std::vector<unsigned int>& frames, const std::string& prefix, int count) {
        frames.clear();
        char path[256];
        for (int i = 0; i < count; i++) {
            sprintf_s(path, 256, "%s\\frame_%03d.png", prefix.c_str(), i);
            frames.push_back(iLoadImage(path, 255, 255, 255));
        }
    }

    void LoadFrames(std::vector<unsigned int>& frames, const std::string& prefix, int count) {
        frames.clear();
        char path[256];
        for (int i = 1; i <= count; i++) {
            sprintf_s(path, 256, "%s (%d).png", prefix.c_str(), i);
            frames.push_back(iLoadImage(path, 255, 255, 255));
        }
    }

    void DrawHealthBar(float x, float y, float current, float maxVal, float r, float g, float b, float width = 100) {
		iSetColor(100, 100, 100);
		iFilledRectangle(x, y, width, 10);
		if (maxVal > 0) {
			float ratio = current / maxVal;
			if (ratio < 0) ratio = 0;
			if (ratio > 1) ratio = 1;
			iSetColor(r, g, b);
			iFilledRectangle(x, y, width * ratio, 10);
		}
		iSetColor(255, 255, 255);
		iRectangle(x, y, width, 10);
	}

public:
    std::string currentMonsterName;

    static FinalBossManager& GetInstance() {
        static FinalBossManager instance;
        return instance;
    }

    void HealPlayer(float percentage) {
        playerHP += playerMaxHP * percentage;
        if (playerHP > playerMaxHP) playerHP = playerMaxHP;
    }

    void Init(std::string monsterName = "Vivi") {
        currentMonsterName = monsterName;
        backgroundTex = iLoadImage("Image//combat render 4.png");

        if (monsterName == "Dawn") {
            LoadFramesZeroIndexed(playerSprintRight, "Image//Monster Images//Player Monsters//Dawn//Dawn Walk right", 25);
            LoadFramesZeroIndexed(playerSprintLeft, "Image//Monster Images//Player Monsters//Dawn//Dawn Walk left", 25);
            LoadFramesZeroIndexed(playerAttackRight, "Image//Monster Images//Player Monsters//Dawn//Dawn Attack right", 25);
            LoadFramesZeroIndexed(playerAttackLeft, "Image//Monster Images//Player Monsters//Dawn//Dawn Attack left", 25);
            LoadFramesZeroIndexed(playerTCAttackRight, "Image//Monster Images//Player Monsters//Dawn//Dawn Special Attack right", 25);
            LoadFramesZeroIndexed(playerTCAttackLeft, "Image//Monster Images//Player Monsters//Dawn//Dawn Special Attack left", 25);
            LoadFramesZeroIndexed(playerJumpRight, "Image//Monster Images//Player Monsters//Dawn//Dawn Walk right", 25);
            LoadFramesZeroIndexed(playerJumpLeft, "Image//Monster Images//Player Monsters//Dawn//Dawn Walk left", 25);
        } else if (monsterName == "Drake") {
            LoadFramesZeroIndexed(playerSprintRight, "Image//Monster Images//Player Monsters//Drake//Drake Fly right", 25);
            LoadFramesZeroIndexed(playerSprintLeft, "Image//Monster Images//Player Monsters//Drake//Drake Fly left", 25);
            LoadFramesZeroIndexed(playerAttackRight, "Image//Monster Images//Player Monsters//Drake//Drake Attack right", 25);
            LoadFramesZeroIndexed(playerAttackLeft, "Image//Monster Images//Player Monsters//Drake//Drake Attack left", 25);
            LoadFramesZeroIndexed(playerTCAttackRight, "Image//Monster Images//Player Monsters//Drake//Drake Special Attack right", 25);
            LoadFramesZeroIndexed(playerTCAttackLeft, "Image//Monster Images//Player Monsters//Drake//Drake Special Attack left", 25);
            LoadFramesZeroIndexed(playerJumpRight, "Image//Monster Images//Player Monsters//Drake//Drake Fly right", 25);
            LoadFramesZeroIndexed(playerJumpLeft, "Image//Monster Images//Player Monsters//Drake//Drake Fly left", 25);
        } else { // Vivi (default)
            LoadFramesZeroIndexed(playerSprintRight, "Image//Monster Images//Player Monsters//Vivi//Vivi Sprint Right", 25);
            LoadFramesZeroIndexed(playerSprintLeft, "Image//Monster Images//Player Monsters//Vivi//Vivi Sprint Left", 25);
            LoadFrames(playerAttackRight, "Image//Monster Images//Player Monsters//Vivi//Vivi Attack Right//vivi_lb", 25);
            LoadFrames(playerAttackLeft, "Image//Monster Images//Player Monsters//Vivi//Vivi Attack Left//vivi_lb", 25);
            LoadFrames(playerTCAttackRight, "Image//Monster Images//Player Monsters//Vivi//Vivi Special Attack Right//vivi_tc", 25);
            LoadFrames(playerTCAttackLeft, "Image//Monster Images//Player Monsters//Vivi//Vivi Special Attack Left//vivi_tc", 25);
            LoadFramesZeroIndexed(playerJumpRight, "Image//Monster Images//Player Monsters//Vivi//Vivi Jump Right", 25);
            LoadFramesZeroIndexed(playerJumpLeft, "Image//Monster Images//Player Monsters//Vivi//Vivi Jump Left", 25);
        }

        // Boss: Mogambo - Left and Right
        LoadFramesZeroIndexed(mogamboWalkL, "Image//Monster Images//Mogambo//Mogambo Walk left", 25);
        LoadFramesZeroIndexed(mogamboWalkR, "Image//Monster Images//Mogambo//Mogambo Walk Right", 25);
        LoadFramesZeroIndexed(mogamboAtkL, "Image//Monster Images//Mogambo//Mogambo Attack left", 25);
        LoadFramesZeroIndexed(mogamboAtkR, "Image//Monster Images//Mogambo//Mogambo Attack right", 25);
        LoadFramesZeroIndexed(mogamboSpAtkL, "Image//Monster Images//Mogambo//Mogambo Special Attack left", 25);
        LoadFramesZeroIndexed(mogamboSpAtkR, "Image//Monster Images//Mogambo//Mogambo Special Attack right", 25);
        LoadFramesZeroIndexed(mogamboSmashL, "Image//Monster Images//Mogambo//Mogambo Ground Smash Attack left", 25);
        LoadFramesZeroIndexed(mogamboSmashR, "Image//Monster Images//Mogambo//Mogambo Ground Smash Attack right", 25);

        playerHP = 500;
        playerMaxHP = 500; // Large HP pool
        playerX = 100;
        basePlayerY = 50;
        playerY = basePlayerY;
        CombatManager::GetInstance().damagePotionUsed = false;
        playerFrame = 0;
        playerAttackFrame = 0;
        playerAnimTimer = 0;
        playerFacingRight = true;
        playerIsMoving = false;
        playerAttackType = 0;
        playerAttackTimer = 0;
        velocityY = 0;
        isJumping = false;

        bossHP = 2000;
        bossMaxHP = 2000; // Big Boss
        bossX = 700;
        bossY = 50;
        bossFrame = 0;
        bossAnimTimer = 0;
        bossFacingRight = false;
        bossAttackType = 0;
        
        playerHitCount = 0;
        redGlowTimer = 0;
        specialAttackTimer = 0;
        groundSmashTimer = 0;
        normalAttackTimer = 2.0f;
        useGroundSmashNext = false;
        hasTakenSpecialDamage = false;

        currentState = FB_PLAYING;
    }

    void MovePlayer(float dx) {
        if (currentState != FB_PLAYING) return;
        // Double movement speed from dx!
        dx *= 2.0f; 

        playerIsMoving = true;
        playerX += dx;
        if (playerX < 0) playerX = 0;
        if (playerX > 900) playerX = 900;

        if (dx > 0) playerFacingRight = true;
        if (dx < 0) playerFacingRight = false;
    }

    void JumpPlayer() {
        if (currentState != FB_PLAYING) return;
        if (!isJumping) {
            isJumping = true;
            velocityY = JUMP_SPEED;
            playerFrame = 0; 
        }
    }

    void PlayerAttack(int type) {
        if (currentState != FB_PLAYING || playerAttackType != 0) return;
        playerAttackType = type; // 1 = LB, 2 = TC
        playerAttackTimer = 0.5f; // half second cast
        playerAttackFrame = 0;

        // Apply Damage if in range
        float dist = fabs(playerX - bossX);
        if (dist <= 300.0f) {
            float baseDmg = 50.0f;
            if (currentMonsterName == "Dawn") baseDmg = 65.0f;
            else if (currentMonsterName == "Drake") baseDmg = 80.0f;

            float dmg = (type == 1) ? baseDmg : baseDmg * 2.0f;
            if (CombatManager::GetInstance().damagePotionUsed) {
                dmg *= 1.5f;
            }
            bossHP -= dmg;
            if (bossHP <= 0) {
                bossHP = 0;
                currentState = FB_VICTORY;
                return;
            }

            // Increment Boss Hit Count tracker
            playerHitCount++;
            if (playerHitCount >= 3) {
                playerHitCount = 0;
                // Trigger Red Glow!
                if (redGlowTimer <= 0 && specialAttackTimer <= 0 && groundSmashTimer <= 0) {
                    redGlowTimer = 2.0f; 
                    hasTakenSpecialDamage = false; // Reset damage flag
                }
            }
        }
    }

    void Update(float deltaTime) {
        if (currentState != FB_PLAYING) return;

        // Player Anim Update
        playerAnimTimer += deltaTime;
        if (playerAnimTimer >= 0.05f) {
            playerAnimTimer = 0;
            if (isJumping) {
                 playerFrame++; // iterate jump frames
            } else if (playerIsMoving) {
                 playerFrame++;
            } else {
                 playerFrame = 0; // idle placeholder
            }
            playerIsMoving = false;
        }

        // Apply Player Jumping Physics
        if (isJumping) {
            playerY += velocityY * deltaTime;
            velocityY -= GRAVITY * deltaTime;
            if (playerY <= basePlayerY) {
                playerY = basePlayerY;
                isJumping = false;
                velocityY = 0;
            }
        }

        if (playerAttackType != 0) {
            playerAttackTimer -= deltaTime;
            int maxAtkFrames = playerFacingRight ? playerAttackRight.size() : playerAttackLeft.size();
            // Scale attack frame visually
            playerAttackFrame = (int)(( (0.5f - playerAttackTimer) / 0.5f) * maxAtkFrames);
            
            if (playerAttackTimer <= 0) {
                playerAttackType = 0;
            }
        }

        bossFacingRight = (playerX > bossX);
        float dist = fabs(playerX - bossX);
        float attackRange = 250.0f;

        // Boss Logic
        if (redGlowTimer > 0) {
            redGlowTimer -= deltaTime;
            if (redGlowTimer <= 0) {
                // start special attack phase depending on alternation
                if (useGroundSmashNext) {
                    groundSmashTimer = 1.2f;
                    bossAttackType = 3; // ground smash
                } else {
                    specialAttackTimer = 2.5f;
                    bossAttackType = 2; // normal special
                }
                useGroundSmashNext = !useGroundSmashNext; // toggle next
                bossFrame = 0;
            }
        } 
        else if (specialAttackTimer > 0) {
            specialAttackTimer -= deltaTime;
            // 70% Max HP drain mechanic
            if (dist <= attackRange && !hasTakenSpecialDamage) {
                playerHP -= (playerMaxHP * 0.70f); // BIG Hit
                hasTakenSpecialDamage = true;
                if (playerHP < 0) playerHP = 0;
                playerX = playerX < bossX ? playerX - 100 : playerX + 100;
                
                if (playerHP <= 0) {
                    currentState = FB_DEFEAT;
                    return;
                }
            }

            if (specialAttackTimer <= 0) {
                bossAttackType = 0;
            }
        } 
        else if (groundSmashTimer > 0) {
            groundSmashTimer -= deltaTime;
            // 40% Ground hit check
            if (!hasTakenSpecialDamage) {
                // If the player is on the ground or barely jumping, they get hit!
                if (playerY <= basePlayerY + 50.0f) {
                     if (currentMonsterName != "Drake") { // Drake flies and is immune to ground smashes
                         playerHP -= (playerMaxHP * 0.40f);
                         hasTakenSpecialDamage = true;
                         if (playerHP <= 0) {
                             playerHP = 0;
                             currentState = FB_DEFEAT;
                             return;
                         }
                     }
                }
            }
            
            if (groundSmashTimer <= 0) {
                bossAttackType = 0;
            }
        }
        else {
            // Normal Walk/Attack AI
            if (dist > attackRange) {
                float speed = 120.0f;
                if (playerX > bossX) bossX += speed * deltaTime;
                else                 bossX -= speed * deltaTime;
                bossAttackType = 0;
            } else {
                normalAttackTimer -= deltaTime;
                bossAttackType = 1; // normal attack
                if (normalAttackTimer <= 0) {
                    playerHP -= 20.0f;
                    normalAttackTimer = 1.0f;
                    if (playerHP <= 0) {
                        playerHP = 0;
                        currentState = FB_DEFEAT;
                        return;
                    }
                }
            }
        }

        // Boss Anim Update
        bossAnimTimer += deltaTime;
        if (bossAnimTimer >= 0.05f) {
            bossAnimTimer = 0;
            bossFrame++;
        }
    }

    void Render() {
        iShowImage(0, 0, 1000, 600, backgroundTex);

        // Render Player
        int pTex = 0;
        if (playerAttackType > 0) {
            std::vector<unsigned int>* atk = playerFacingRight ? (playerAttackType == 1 ? &playerAttackRight : &playerTCAttackRight) : 
                                                                 (playerAttackType == 1 ? &playerAttackLeft : &playerTCAttackLeft);
            if (!atk->empty()) {
                pTex = (*atk)[playerAttackFrame % atk->size()];
            }
        } else {
            // Priority: Jump frames over Walk frames
            std::vector<unsigned int>* spr;
            if (isJumping) {
                 spr = playerFacingRight ? &playerJumpRight : &playerJumpLeft;
            } else {
                 spr = playerFacingRight ? &playerSprintRight : &playerSprintLeft;
            }
            if (!spr->empty()) {
                pTex = (*spr)[playerFrame % spr->size()];
            }
        }
        if (pTex > 0) iShowImage(playerX, playerY, 150, 150, pTex);

        // Player HP
        DrawHealthBar(playerX + 25, playerY + 160, playerHP, playerMaxHP, 0, 255, 0);

        // Render Boss
        int bTex = 0;
        std::vector<unsigned int>* bAnim = bossFacingRight ? &mogamboWalkR : &mogamboWalkL;

        if (bossAttackType == 1) bAnim = bossFacingRight ? &mogamboAtkR : &mogamboAtkL;
        if (bossAttackType == 2) bAnim = bossFacingRight ? &mogamboSpAtkR : &mogamboSpAtkL;
        if (bossAttackType == 3) bAnim = bossFacingRight ? &mogamboSmashR : &mogamboSmashL;

        if (!bAnim->empty()) {
            bTex = (*bAnim)[bossFrame % bAnim->size()];
        }

        if (bTex > 0) {
            // Apply Red Glow Tint
            if (redGlowTimer > 0) {
                // Modifying tint might not be natively supported by standard iShowImage.
                // We'll emulate it by rendering a red rectangle behind him or using iSetColor if the lib supports tint.
                // Note: iGraphics often tints the NEXT drawn image if we use iSetColor? No, images are usually opaque natively.
                // Best fallback for "red glow" is drawing a translucent red rectangle (or filled circle) over/around him:
                
                iShowImage(bossX, bossY, 300, 300, bTex); // Normal draw
                // Optional native iGraphics transparency attempt:
                // We can just rely on the user understanding this is the red phase via text & a red pulsing background glow.
                iSetColor(255, 0, 0);
                iText(bossX + 50, bossY + 350, "MOGAMBO ENRAGED!", (void*)0x0006);
            } else {
                iShowImage(bossX, bossY, 300, 300, bTex);
            }
        }

        // Boss HP
        DrawHealthBar(bossX + 50, bossY + 320, bossHP, bossMaxHP, 255, 0, 0, 200);

        // Game Over overlays
        if (currentState == FB_VICTORY) {
            iSetColor(0, 255, 0);
            iText(300, 400, "FINAL BOSS DEFEATED! YOU WIN!", (void*)0x0006);
            iSetColor(255, 255, 255);
            iText(350, 350, "Press ESC to return to Title", (void*)0x0008);
        } else if (currentState == FB_DEFEAT) {
            iSetColor(255, 0, 0);
            iText(400, 400, "DEFEAT...", (void*)0x0006);
            iSetColor(255, 255, 255);
            iText(350, 350, "Press SPACE to Restart", (void*)0x0008);
        }
    }

    void HandleInput(unsigned char key) {
        if (currentState == FB_DEFEAT) {
            if (key == ' ') Init();
        } else if (currentState == FB_VICTORY) {
            if (key == 27) { // ESC -> back to menu? Let's assume handled outside.
            }
        }
    }
};

#endif
