#ifndef COMBATMANAGER_H
#define COMBATMANAGER_H

#include <string>
#include <vector>
#include <cstdio>
#include <algorithm>
#include <cmath>
#include "audio.h"

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
	bool facingRight; // Direction flag: true = facing right

	// Animations (RIGHT-facing for player; LEFT-facing for enemies by default)
	std::vector<unsigned int> idleFrames;
	std::vector<unsigned int> walkFrames;
	std::vector<unsigned int> attack1Frames; // Lightning Blast
	std::vector<unsigned int> attack2Frames; // Thunder Crash

	// Alternative direction frames
	// For player: left-facing versions
	// For enemies: right-facing versions (enemy walking/attacking toward player on their right)
	std::vector<unsigned int> idleFramesAlt;
	std::vector<unsigned int> walkFramesAlt;
	std::vector<unsigned int> attack1FramesAlt;
	std::vector<unsigned int> attack2FramesAlt;

	int currentFrame;
	float animTimer;
	int currentAttackType; // 0=None, 1=LB, 2=TC
	bool isMoving; // Track movement state
	float attackTimer; // Unique attack timer per enemy instance
	int attacksPerformed;

	Monster() : maxHealth(100), currentHealth(100), attackPower(10), x(0), y(0), textureID(0), facingRight(true), currentFrame(0), animTimer(0), currentAttackType(0), isMoving(false), attackTimer(0), attacksPerformed(0) {}
};

class CombatManager {
public:
	int lives;
	float bonusStrength;
	bool dawnUnlocked;
	bool drakeUnlocked;
	int lastTowerPlayed;

	// Endgame tracking
	bool tower1Cleared;
	bool tower2Cleared;
	bool tower3Cleared;
	bool allTowersCleared;
	bool endgameRunnerDone;   // collected 10 crystals after all towers cleared
	bool endgameMemoryDone;   // completed memory game after all towers cleared

	// Task / Phase tracking
	bool phase1RewardGiven;
	bool phase2RewardGiven;
	bool phase3RewardGiven;
	int phase2CoinTarget;
	bool phase2MemoryDone;

	// Tracks if the player was forced here by losing all lives (not voluntary visit)
	bool sentToWildAreaByLoss;

	// Shop Potions Inventory
	int healPotionCount;
	int damagePotionCount;
	bool damagePotionUsed;

	static CombatManager& GetInstance() {
		static CombatManager instance;
		return instance;
	}

	void HealPlayer(float percentage) {
		playerMonster.currentHealth += playerMonster.maxHealth * percentage;
		if (playerMonster.currentHealth > playerMonster.maxHealth) {
			playerMonster.currentHealth = playerMonster.maxHealth;
		}
	}

	bool IsVictory() const {
		return currentState == VICTORY;
	}

	bool IsDefeat() const {
		return currentState == DEFEAT;
	}

	void InitCombat() {
		// Reset states for new combat
		damagePotionUsed = false;
		currentWave = 1;
		currentState = COMBAT_START;
		lastTowerPlayed = 1;

		// Init Player
		playerMonster.name = "Vivi";
		playerMonster.textureIdle = "Image//lightning_monster_idle.bmp"; // Fallback
		playerMonster.textureID = iLoadImage(playerMonster.textureIdle.c_str(), 255, 255, 255);

		// Load Animations for Vivi (right-facing = default, left-facing = alt)
		LoadAnimationFrames(playerMonster.idleFrames,     "Image//Monster Images//Player Monsters//Vivi//Vivi Idle Right//vivo_standing", 16);
		LoadAnimationFrames(playerMonster.walkFrames,     "Image//Monster Images//Player Monsters//Vivi//Vivi Walk Right//vivi_walk", 25);
		LoadAnimationFrames(playerMonster.attack1Frames,  "Image//Monster Images//Player Monsters//Vivi//Vivi Attack Right//vivi_lb", 25);
		LoadAnimationFrames(playerMonster.attack2Frames,  "Image//Monster Images//Player Monsters//Vivi//Vivi Special Attack Right//vivi_tc", 25);
		LoadAnimationFrames(playerMonster.idleFramesAlt,  "Image//Monster Images//Player Monsters//Vivi//Vivi Idle Left//vivo_standing", 16);
		LoadAnimationFrames(playerMonster.walkFramesAlt,  "Image//Monster Images//Player Monsters//Vivi//Vivi Walk Left//vivi_walk", 25);
		LoadAnimationFrames(playerMonster.attack1FramesAlt, "Image//Monster Images//Player Monsters//Vivi//Vivi Attack Left//vivi_lb", 25);
		LoadAnimationFrames(playerMonster.attack2FramesAlt, "Image//Monster Images//Player Monsters//Vivi//Vivi Special Attack Left//vivi_tc", 25);
		playerMonster.facingRight = true;

		// Load Animations for Enemy (Black) — left=default, right=alt
		LoadAnimationFrames(enemyTemplate.idleFrames,    "Image//Monster Images//Black//Black Walk Left//black_running", 16);
		LoadAnimationFrames(enemyTemplate.walkFramesAlt, "Image//Monster Images//Black//Black Walk Right//black_running", 16);
		enemyTemplate.facingRight = false;

		playerMonster.y = PLAYER_START_Y;

		isTower3 = false;
		bgScrollOffset = 0.0f;
		backgroundTextureID = iLoadImage("Image//combact_background.bmp");
		SetupWave(currentWave);
	}

	void InitTower3() {
		// Optional: specific init for Tower 3 if needed
		damagePotionUsed = false;
		currentWave = 1;
		currentState = COMBAT_START;
		lastTowerPlayed = 3;

		// Init Player
		playerMonster.name = "Vivi";
		playerMonster.textureIdle = "Image//lightning_monster_idle.bmp"; // Fallback
		playerMonster.textureID = iLoadImage(playerMonster.textureIdle.c_str(), 255, 255, 255);

		// Load Animations for Vivi (right-facing = default, left-facing = alt)
		LoadAnimationFrames(playerMonster.idleFrames,     "Image//Monster Images//Player Monsters//Vivi//Vivi Idle Right//vivo_standing", 16);
		LoadAnimationFrames(playerMonster.walkFrames,     "Image//Monster Images//Player Monsters//Vivi//Vivi Walk Right//vivi_walk", 25);
		LoadAnimationFrames(playerMonster.attack1Frames,  "Image//Monster Images//Player Monsters//Vivi//Vivi Attack Right//vivi_lb", 25);
		LoadAnimationFrames(playerMonster.attack2Frames,  "Image//Monster Images//Player Monsters//Vivi//Vivi Special Attack Right//vivi_tc", 25);
		LoadAnimationFrames(playerMonster.idleFramesAlt,  "Image//Monster Images//Player Monsters//Vivi//Vivi Idle Left//vivo_standing", 16);
		LoadAnimationFrames(playerMonster.walkFramesAlt,  "Image//Monster Images//Player Monsters//Vivi//Vivi Walk Left//vivi_walk", 25);
		LoadAnimationFrames(playerMonster.attack1FramesAlt, "Image//Monster Images//Player Monsters//Vivi//Vivi Attack Left//vivi_lb", 25);
		LoadAnimationFrames(playerMonster.attack2FramesAlt, "Image//Monster Images//Player Monsters//Vivi//Vivi Special Attack Left//vivi_tc", 25);
		playerMonster.facingRight = true;

		// Load Animations for Enemy Template (Black) — left=default, right=alt
		LoadAnimationFrames(enemyTemplate.idleFrames,    "Image//Monster Images//Black//Black Walk Left//black_running", 16);
		LoadAnimationFrames(enemyTemplate.walkFramesAlt, "Image//Monster Images//Black//Black Walk Right//black_running", 16);
		enemyTemplate.facingRight = false;

		playerMonster.x = PLAYER_START_X;
		playerMonster.y = PLAYER_START_Y;

		isTower3 = true;
		bgScrollOffset = 0.0f;
		backgroundTextureID = iLoadImage("Image//combat render3.png");
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

	void LoadAnimationFramesZeroIndexed(std::vector<unsigned int>& frames, std::string prefix, int count) {
		frames.clear();
		char path[256];
		for (int i = 0; i < count; i++) {
			sprintf_s(path, 256, "%s\\frame_%03d.png", prefix.c_str(), i);
			frames.push_back(iLoadImage(path, 255, 255, 255));
		}
	}

	void InitCombatWithDawn() {
		currentWave = 1;
		currentState = COMBAT_START;
		lives = 3; // Restore lives fully!
		lastTowerPlayed = 1;

		// Init Player as Dawn
		playerMonster.name = "Dawn";
		playerMonster.textureIdle = ""; 

		// Load Animations for Dawn (right=default, left=alt)
		LoadAnimationFramesZeroIndexed(playerMonster.idleFrames,      "Image//Monster Images//Player Monsters//Dawn//Dawn Walk right", 25);
		LoadAnimationFramesZeroIndexed(playerMonster.walkFrames,      "Image//Monster Images//Player Monsters//Dawn//Dawn Walk right", 25);
		LoadAnimationFramesZeroIndexed(playerMonster.attack1Frames,   "Image//Monster Images//Player Monsters//Dawn//Dawn Attack right", 25);
		LoadAnimationFramesZeroIndexed(playerMonster.attack2Frames,   "Image//Monster Images//Player Monsters//Dawn//Dawn Special Attack right", 25);
		LoadAnimationFramesZeroIndexed(playerMonster.idleFramesAlt,   "Image//Monster Images//Player Monsters//Dawn//Dawn Walk left", 25);
		LoadAnimationFramesZeroIndexed(playerMonster.walkFramesAlt,   "Image//Monster Images//Player Monsters//Dawn//Dawn Walk left", 25);
		LoadAnimationFramesZeroIndexed(playerMonster.attack1FramesAlt, "Image//Monster Images//Player Monsters//Dawn//Dawn Attack left", 25);
		LoadAnimationFramesZeroIndexed(playerMonster.attack2FramesAlt, "Image//Monster Images//Player Monsters//Dawn//Dawn Special Attack left", 25);
		playerMonster.facingRight = true;

		if (!playerMonster.idleFrames.empty()) {
			playerMonster.textureID = playerMonster.idleFrames[0];
		}

		// Load Animations for Enemy (Black) — left=default, right=alt
		LoadAnimationFrames(enemyTemplate.idleFrames,    "Image//Monster Images//Black//Black Walk Left//black_running", 16);
		LoadAnimationFrames(enemyTemplate.walkFramesAlt, "Image//Monster Images//Black//Black Walk Right//black_running", 16);
		enemyTemplate.facingRight = false;

		playerMonster.y = PLAYER_START_Y;
		playerMonster.x = PLAYER_START_X;

		isTower3 = false;
		bgScrollOffset = 0.0f;
		backgroundTextureID = iLoadImage("Image//combact_background.bmp");
		SetupWave(currentWave);
	}

	void InitTower3WithDrake() {
		currentWave = 1;
		currentState = COMBAT_START;
		lives = 3; // Restore lives fully!
		lastTowerPlayed = 3;

		// Init Player as Drake
		playerMonster.name = "Drake";
		playerMonster.textureIdle = ""; 

		// Load Animations for Drake (right=default, left=alt, uses Fly animation for walk)
		LoadAnimationFramesZeroIndexed(playerMonster.idleFrames,      "Image//Monster Images//Player Monsters//Drake//Drake Fly right", 25);
		LoadAnimationFramesZeroIndexed(playerMonster.walkFrames,      "Image//Monster Images//Player Monsters//Drake//Drake Fly right", 25);
		LoadAnimationFramesZeroIndexed(playerMonster.attack1Frames,   "Image//Monster Images//Player Monsters//Drake//Drake Attack right", 25);
		LoadAnimationFramesZeroIndexed(playerMonster.attack2Frames,   "Image//Monster Images//Player Monsters//Drake//Drake Special Attack right", 25);
		LoadAnimationFramesZeroIndexed(playerMonster.idleFramesAlt,   "Image//Monster Images//Player Monsters//Drake//Drake Fly left", 25);
		LoadAnimationFramesZeroIndexed(playerMonster.walkFramesAlt,   "Image//Monster Images//Player Monsters//Drake//Drake Fly left", 25);
		LoadAnimationFramesZeroIndexed(playerMonster.attack1FramesAlt, "Image//Monster Images//Player Monsters//Drake//Drake Attack left", 25);
		LoadAnimationFramesZeroIndexed(playerMonster.attack2FramesAlt, "Image//Monster Images//Player Monsters//Drake//Drake Special Attack left", 25);
		playerMonster.facingRight = true;

		if (!playerMonster.idleFrames.empty()) {
			playerMonster.textureID = playerMonster.idleFrames[0];
		}

		// Load Animations for Enemy (Black) — left=default, right=alt
		LoadAnimationFrames(enemyTemplate.idleFrames,    "Image//Monster Images//Black//Black Walk Left//black_running", 16);
		LoadAnimationFrames(enemyTemplate.walkFramesAlt, "Image//Monster Images//Black//Black Walk Right//black_running", 16);
		enemyTemplate.facingRight = false;

		playerMonster.x = PLAYER_START_X;
		playerMonster.y = PLAYER_START_Y;

		isTower3 = true;
		bgScrollOffset = 0.0f;
		backgroundTextureID = iLoadImage("Image//combat render3.png");
		SetupWave(currentWave);
	}

	void SetupWave(int waveIndex) {
		float towerBonusHP = isTower3 ? 100.0f : 0.0f;
		float towerBonusAtk = isTower3 ? 40.0f : 0.0f;
		float dawnBonusAtk = (playerMonster.name == "Dawn") ? 15.0f : 0.0f; // Give Dawn an attack bonus!
		float drakeBonusAtk = (playerMonster.name == "Drake") ? 30.0f : 0.0f; // Give Drake a larger bonus!

		// Apply strength bonus
		playerMonster.attackPower = 40.0f + towerBonusAtk + dawnBonusAtk + drakeBonusAtk + bonusStrength + (20.0f * (waveIndex - 1));
		playerMonster.maxHealth = 100.0f + towerBonusHP + (50.0f * (waveIndex - 1));
		playerMonster.currentHealth = playerMonster.maxHealth;

		// Reset player position
		playerMonster.x = PLAYER_START_X;
		playerMonster.y = PLAYER_START_Y;

		enemyTemplate.name = "Black"; // Default for Tower 1
		enemyTemplate.textureIdle = ""; 

		if (isTower3) {
			std::string folderName = "Rift";
			std::string walkFolderName = "Rift Walking left";
			if (waveIndex == 2) {
				folderName = "Rooter";
				walkFolderName = "Rooter Walking left";
			} else if (waveIndex == 3) {
				folderName = "Slyther";
				walkFolderName = "Slyther Walk left";
			}

			enemyTemplate.name = folderName;

			std::string prefixWalk  = "Image//Monster Images//" + folderName + "//" + walkFolderName;
			std::string prefixAtk1  = "Image//Monster Images//" + folderName + "//" + folderName + " Attack left";
			std::string prefixAtk2  = "Image//Monster Images//" + folderName + "//" + folderName + " Special Attack left";
			// Right-facing alternatives (enemy facing right when player is to their right)
			std::string prefixWalkR = "Image//Monster Images//" + folderName + "//" + folderName + " Walk right";
			// Slyther uses "Walking right" — work-around by checking folderName
			if (folderName == "Slyther") prefixWalkR = "Image//Monster Images//Slyther//Slyther Walking right";
			std::string prefixAtk1R = "Image//Monster Images//" + folderName + "//" + folderName + " Attack right";
			std::string prefixAtk2R = "Image//Monster Images//" + folderName + "//" + folderName + " Special Attack right";
			LoadAnimationFramesZeroIndexed(enemyTemplate.idleFrames,     prefixWalk,  25);
			LoadAnimationFramesZeroIndexed(enemyTemplate.attack1Frames,  prefixAtk1,  25);
			LoadAnimationFramesZeroIndexed(enemyTemplate.attack2Frames,  prefixAtk2,  25);
			LoadAnimationFramesZeroIndexed(enemyTemplate.idleFramesAlt,  prefixWalkR, 25);
			LoadAnimationFramesZeroIndexed(enemyTemplate.attack1FramesAlt, prefixAtk1R, 25);
			LoadAnimationFramesZeroIndexed(enemyTemplate.attack2FramesAlt, prefixAtk2R, 25);
			enemyTemplate.walkFrames    = enemyTemplate.idleFrames;
			enemyTemplate.walkFramesAlt = enemyTemplate.idleFramesAlt;
			enemyTemplate.facingRight = false;
		} else {
			// Tower 1: generic Black entity
			enemyTemplate.name = "Black";
			LoadAnimationFrames(enemyTemplate.idleFrames,    "Image//Monster Images//Black//Black Walk Left//black_running", 16);
			LoadAnimationFrames(enemyTemplate.walkFramesAlt, "Image//Monster Images//Black//Black Walk Right//black_running", 16);
			enemyTemplate.facingRight = false;
		}

		// Scale difficulty
		enemyTemplate.maxHealth = 80.0f + (100.0f * waveIndex); // Scaled appropriately
		enemyTemplate.attackPower = 8.0f + (5.0f * waveIndex);

		if (isTower3) {
			enemyTemplate.attackPower *= 1.75f; // Increase damage output specifically for Tower 3 (Balanced)
		}

		activeEnemies.clear();
		enemiesRemainingToSpawn = isTower3 ? 3 : 1; // Tower 3 gets 3 enemies, Tower 2 gets 1
		nextEnemySpawnTimer = 0.0f; // Spawn the first enemy immediately

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

		// Update facing direction
		if (dx > 0) playerMonster.facingRight = true;
		else if (dx < 0) playerMonster.facingRight = false;

		playerMonster.x += dx;
		playerMonster.isMoving = true; // Set moving flag
		
		if (isTower3) {
			bgScrollOffset -= dx * 0.4f; // Subtle scroll
			
            // Slide enemies relative to camera panning so they stay anchored to the floor
			for (size_t i = 0; i < activeEnemies.size(); i++) {
				activeEnemies[i].x -= dx * 0.4f;
			}
		}

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
			}
			else if (type == 2) { // F: Thunder Crash
				if (currentWave < 3) return;
				currentAttackName = "THUNDER CRASH!";
				currentAttackDamage = playerMonster.attackPower * 2.0f;
			}

            if (damagePotionUsed) {
                currentAttackDamage *= 1.5f;
            }

            AudioSystem::PlaySFX(playerMonster.name, type);

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
		float frameDuration = 0.05f; // Smoother animation

		if (currentState == PLAYER_ATTACK) {
			// Attack Animation (Play Once)
			if (playerMonster.animTimer >= frameDuration) {
				playerMonster.animTimer = 0;
				playerMonster.currentFrame++;

				// Type 1: LB
				if (playerMonster.currentAttackType == 1) {
					if (playerMonster.currentFrame >= (int)playerMonster.attack1Frames.size()) {
						playerMonster.currentFrame = (int)playerMonster.attack1Frames.size() - 1; // Hold last frame?
					}
				}
				// Type 2: TC
				else if (playerMonster.currentAttackType == 2) {
					if (playerMonster.currentFrame >= (int)playerMonster.attack2Frames.size()) {
						playerMonster.currentFrame = (int)playerMonster.attack2Frames.size() - 1;
					}
				}
			}
		}
		else {
			// Idle/Walk Animation (Loop)
			if (playerMonster.animTimer >= frameDuration) {
				playerMonster.animTimer = 0;
				playerMonster.currentFrame++;

				int maxFrames = playerMonster.isMoving ? (int)playerMonster.walkFrames.size() : (int)playerMonster.idleFrames.size();
				if (playerMonster.currentFrame >= maxFrames) {
					playerMonster.currentFrame = 0;
				}

				// Reset moving flag after processing frame (assuming Update called after moves)
				// Actually, MovePlayer calls happen in fixedUpdate which might run multiple times or not at all before Render
				// A better approach is to decay a timer. Or just reset it here if we assume 1 render per logic step.
				// For safety with iGraphics structure:
				playerMonster.isMoving = false;
			}
		}

		// --- Enemy Animation Update ---
		for (size_t i = 0; i < activeEnemies.size(); i++) {
			activeEnemies[i].animTimer += deltaTime;
			if (activeEnemies[i].animTimer >= frameDuration) {
				activeEnemies[i].animTimer = 0;
				activeEnemies[i].currentFrame++;

				if (activeEnemies[i].currentAttackType == 1 && !activeEnemies[i].attack1Frames.empty()) {
					if (activeEnemies[i].currentFrame >= (int)activeEnemies[i].attack1Frames.size()) {
						activeEnemies[i].currentAttackType = 0; // return to walk
						activeEnemies[i].currentFrame = 0;
					}
				} else if (activeEnemies[i].currentAttackType == 2 && !activeEnemies[i].attack2Frames.empty()) {
					if (activeEnemies[i].currentFrame >= (int)activeEnemies[i].attack2Frames.size()) {
						activeEnemies[i].currentAttackType = 0;
						activeEnemies[i].currentFrame = 0;
					}
				} else {
					if (!activeEnemies[i].idleFrames.empty() && activeEnemies[i].currentFrame >= (int)activeEnemies[i].idleFrames.size()) {
						activeEnemies[i].currentFrame = 0;
					}
				}
			}
		}

		// --- Staggered Spawning ---
		if (enemiesRemainingToSpawn > 0 && currentState != WAVE_CLEAR && currentState != VICTORY && currentState != DEFEAT) {
			nextEnemySpawnTimer -= deltaTime;
			if (nextEnemySpawnTimer <= 0) {
				Monster newEnemy = enemyTemplate;
				newEnemy.currentHealth = newEnemy.maxHealth;
				newEnemy.x = 1000.0f; // Spawn off-screen to the right
				newEnemy.y = ENEMY_START_Y;
				newEnemy.attackTimer = (rand() % 100) / 100.0f;
				activeEnemies.push_back(newEnemy);

				enemiesRemainingToSpawn--;
				nextEnemySpawnTimer = 4.0f; // 4 seconds between staggered enemy spawns
			}
		}
		// ------------------------

		// WAVE CLEAR LOGIC
		if (currentState == WAVE_CLEAR) {
			// Fast forward background (Tower 3 only)
			if (isTower3) {
				bgScrollOffset -= 400.0f * deltaTime;
			}
			
			// Take player back to starting position
			if (playerMonster.x > PLAYER_START_X) {
				playerMonster.x -= 300.0f * deltaTime; // Move left quickly
				playerMonster.isMoving = true;        // Play run animation
				if (playerMonster.x < PLAYER_START_X) {
					playerMonster.x = PLAYER_START_X;
				}
			}

			if (stateTimer > 2.0f) { // Wait 2 seconds before next wave
				SetupWave(currentWave);
			}
			return; // Don't process other logic
		}

		// Real-time Enemy Attacks and Vector Removal checks
		if (currentState != WAVE_CLEAR && currentState != VICTORY && currentState != DEFEAT) {
			for (size_t i = 0; i < activeEnemies.size(); i++) {
				activeEnemies[i].attackTimer -= deltaTime;
				float dx = playerMonster.x - activeEnemies[i].x;
				float dist = fabs(dx);

				// If in range and cooldown ready, ATTACK!
				if (dist < 150.0f && activeEnemies[i].attackTimer <= 0) {
					activeEnemies[i].attacksPerformed++;
					float damageToDeal = activeEnemies[i].attackPower;

					if (activeEnemies[i].attacksPerformed % 4 == 0) {
						damageToDeal *= 2.5f; // "significantly more damage"
						activeEnemies[i].currentAttackType = 2; 
						AudioSystem::PlaySFX(activeEnemies[i].name, 2);
					} else {
						damageToDeal *= 0.6f; // "significantly weaker"
						activeEnemies[i].currentAttackType = 1;
						AudioSystem::PlaySFX(activeEnemies[i].name, 1);
					}
					
					activeEnemies[i].currentFrame = 0; // Reset anim
					playerMonster.currentHealth -= damageToDeal;
					activeEnemies[i].attackTimer = 1.5f; // Limit attack speed

					if (playerMonster.currentHealth <= 0) {
						playerMonster.currentHealth = 0;
						currentState = DEFEAT;
						return;
					}
				}
			}
		}

		// Enemy AI: Move towards player to fight
		if (currentState != WAVE_CLEAR && currentState != VICTORY && currentState != DEFEAT) {
			for (size_t i = 0; i < activeEnemies.size(); i++) {
				float dx = playerMonster.x - activeEnemies[i].x;
				float distance = fabs(dx);
				float stopDistance = 150.0f; // All enemies converge directly to fight, no queueing!

				// Update enemy facing direction based on player position
				activeEnemies[i].facingRight = (playerMonster.x > activeEnemies[i].x);

				if (distance > stopDistance) {
					float speed = 40.0f + (i * 20.0f); // Enemies walk at different dynamic speeds
					if (dx > 0) activeEnemies[i].x += speed * deltaTime;
					else activeEnemies[i].x -= speed * deltaTime;
				}

				if (activeEnemies[i].x < 0) activeEnemies[i].x = 0;
				// Clamping upper bounds removed so monsters can spawn outside screen edge at x=1000 and walk in naturally
			}
		}

		switch (currentState) {
		case PLAYER_TURN:
			break;

		case PLAYER_ATTACK:
			if (stateTimer > 0.8f) { // Attack impact moment
				playerHit = false; // Reset hit flag

				// AoE logic: Check all enemies
				for (int i = (int)activeEnemies.size() - 1; i >= 0; i--) {
					float dx = playerMonster.x - activeEnemies[i].x;
					float dist = fabs(dx);

					if (dist < 250.0f) { // Range
						activeEnemies[i].currentHealth -= currentAttackDamage;
						playerHit = true;

						if (activeEnemies[i].currentHealth <= 0) {
							activeEnemies.erase(activeEnemies.begin() + i); // Monster disappears!
						}
					}
				}

				if (activeEnemies.empty() && enemiesRemainingToSpawn <= 0) {
					EndWave();
					return; // Stop processing
				}

				currentState = PLAYER_TURN; // Back to player control immediately
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
		if (isTower3) {
			float wrapOffset = fmod(bgScrollOffset, 1100.0f);
			if (wrapOffset > 0) wrapOffset -= 1100.0f;
			iShowImage((int)wrapOffset, 0, 1100, 600, backgroundTextureID);
			iShowImage((int)wrapOffset + 1100, 0, 1100, 600, backgroundTextureID);
		} else {
			iShowImage(0, 0, 1000, 600, backgroundTextureID);
		}

		// Draw Player (Animated) — pick direction-appropriate frame set
		unsigned int tex = playerMonster.textureID;
		int drawW = 100;
		int drawH = 100;

		int drawX = (int)playerMonster.x;
		int drawY = (int)playerMonster.y;

		// Helper lambdas: choose right or alt (left) frame set based on facingRight
		std::vector<unsigned int>& pAtk1 = playerMonster.facingRight ? playerMonster.attack1Frames : playerMonster.attack1FramesAlt;
		std::vector<unsigned int>& pAtk2 = playerMonster.facingRight ? playerMonster.attack2Frames : playerMonster.attack2FramesAlt;
		std::vector<unsigned int>& pIdle = playerMonster.facingRight ? playerMonster.idleFrames    : playerMonster.idleFramesAlt;
		std::vector<unsigned int>& pWalk = playerMonster.facingRight ? playerMonster.walkFrames    : playerMonster.walkFramesAlt;

		if (currentState == PLAYER_ATTACK) {
			if (playerMonster.name == "Vivi" || playerMonster.name == "") {
				drawW = 150;
				drawH = 150;
				drawX -= 25;
				drawY -= 25;
			}

			if (playerMonster.currentAttackType == 1 && !pAtk1.empty()) {
				tex = pAtk1[playerMonster.currentFrame % pAtk1.size()];
			}
			else if (playerMonster.currentAttackType == 2 && !pAtk2.empty()) {
				tex = pAtk2[playerMonster.currentFrame % pAtk2.size()];
			}
		}
		else {
			// Idle/Walk
			std::vector<unsigned int>* currentAnim = &pIdle;
			if (playerMonster.isMoving && !pWalk.empty()) {
				currentAnim = &pWalk;
			}

			if (!currentAnim->empty()) {
				tex = (*currentAnim)[playerMonster.currentFrame % currentAnim->size()];
			}
		}

		iShowImage(drawX, drawY, drawW, drawH, tex);
		DrawHealthBar(playerMonster.x + 20, playerMonster.y + 250, playerMonster.currentHealth, playerMonster.maxHealth, 0, 255, 0);
		// HP Text
		char hpBuffer[32];
		sprintf_s(hpBuffer, 32, "HP: %d/%d", (int)playerMonster.currentHealth, (int)playerMonster.maxHealth);
		iSetColor(255, 255, 255);
		iText(playerMonster.x + 20, playerMonster.y + 265, hpBuffer, (void*)0x0008);

		// Draw Enemies (Loop through active enemies) — direction-aware
		for (size_t i = 0; i < activeEnemies.size(); i++) {
			int ew = 100, eh = 100;

			// Pick left-facing (default) or right-facing (alt) set based on enemy direction
			std::vector<unsigned int>& eWalk = activeEnemies[i].facingRight ? activeEnemies[i].walkFramesAlt  : activeEnemies[i].idleFrames;
			std::vector<unsigned int>& eAtk1 = activeEnemies[i].facingRight ? activeEnemies[i].attack1FramesAlt : activeEnemies[i].attack1Frames;
			std::vector<unsigned int>& eAtk2 = activeEnemies[i].facingRight ? activeEnemies[i].attack2FramesAlt : activeEnemies[i].attack2Frames;

			std::vector<unsigned int>* currentAnim = &eWalk;
			if (activeEnemies[i].currentAttackType == 1 && !eAtk1.empty()) {
				currentAnim = &eAtk1;
			} else if (activeEnemies[i].currentAttackType == 2 && !eAtk2.empty()) {
				currentAnim = &eAtk2;
			}

			if (!currentAnim->empty()) {
				unsigned int eTex = (*currentAnim)[activeEnemies[i].currentFrame % currentAnim->size()];
				iShowImage((int)activeEnemies[i].x, (int)activeEnemies[i].y, ew, eh, eTex);
			}

			// Enemy HP bar
			float barY = activeEnemies[i].y + 200 + (35 * i);
			DrawHealthBar(activeEnemies[i].x + 20, barY, activeEnemies[i].currentHealth, activeEnemies[i].maxHealth, 255, 0, 0);
			// HP Text
			char ehpBuf[32];
			sprintf_s(ehpBuf, 32, "HP: %d/%d", (int)activeEnemies[i].currentHealth, (int)activeEnemies[i].maxHealth);
			iSetColor(255, 255, 255);
			iText(activeEnemies[i].x + 20, barY + 15, ehpBuf, (void*)0x0008);
			
			// Simple name identifier
			iSetColor(255, 100, 100);
			iText(activeEnemies[i].x + 20, barY - 15, enemyTemplate.name.c_str(), (void*)0x0008);
		}

		// Attack Feedback
		if (attackFeedbackTimer > 0) {
			iSetColor(255, 0, 0);
			iText(playerMonster.x, playerMonster.y + 310, currentAttackName.c_str(), (void*)0x0008);
		}

		// Wave Notification and Clear
		if (currentState == WAVE_CLEAR) {
			iSetColor(0, 255, 0);
			iText(400, 500, "WAVE CLEARED!", (void*)0x0005);
		}
		else if (waveMessageTimer > 0) {
			iSetColor(255, 215, 0);
			char waveMsg[64];
			if (currentWave == 3) {
				sprintf_s(waveMsg, 64, "WAVE 3: THUNDER CRASH UNLOCKED!");
			}
			else {
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
			iSetColor(255, 255, 255);
			iText(320, 450, "Press ESC to return to map", (void*)0x0005);
		}
		else if (currentState == DEFEAT) {
			iSetColor(255, 0, 0);
			iText(400, 500, "DEFEAT...", (void*)0x0006);
			iSetColor(255, 255, 255);
			iText(350, 450, "Click to Try Again", (void*)0x0005);
		}
		else {
			char statusBuf[64];
			sprintf_s(statusBuf, 64, "Wave: %d / %d", currentWave, TOTAL_WAVES);
			iSetColor(255, 255, 255);
			iText(400, 550, statusBuf, (void*)0x0008);
		}

		// Controls hint
		iSetColor(180, 180, 180);
		if (currentWave < TOTAL_WAVES) {
			iText(200, 580, "A/D: Move   W: Attack   F: Thunder Crash (Wave 3)   ESC: Exit", (void*)0x0008);
		} else {
			iText(200, 580, "A/D: Move   W: Attack   F: Thunder Crash   ESC: Exit", (void*)0x0008);
		}

		// Draw Potion Hotkeys
		char potText[100];
		sprintf_s(potText, sizeof(potText), "H: Heal (%d)   X: Damage (%d)", healPotionCount, damagePotionCount);
		iSetColor(200, 200, 255);
		iText(200, 560, potText, (void*)0x0008);

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
			sentToWildAreaByLoss = true; // Player lost all lives
			gameState = 5; // Kick back to wild area selection options
			return;
		}

		playerMonster.currentHealth = playerMonster.maxHealth;
		SetupWave(currentWave);
	}

	void RestoreLives() {
		lives = 4;
		sentToWildAreaByLoss = false; // Cleared after life restoration
	}

	void AddStrengthBonus(float bonus) {
		bonusStrength += bonus;
	}

private:
	CombatManager() :
		lives(3),
		bonusStrength(0),
		dawnUnlocked(false),
		drakeUnlocked(false),
		lastTowerPlayed(1),
		tower1Cleared(false),
		tower2Cleared(false),
		tower3Cleared(false),
		allTowersCleared(false),
		endgameRunnerDone(false),
		endgameMemoryDone(false),
		phase1RewardGiven(false),
		phase2RewardGiven(false),
		phase3RewardGiven(false),
		phase2CoinTarget(0),
		phase2MemoryDone(false),
		sentToWildAreaByLoss(false),
		healPotionCount(0),
		damagePotionCount(0),
		damagePotionUsed(false),
		TOTAL_WAVES(3),
		PLAYER_START_X(100),
		PLAYER_START_Y(50),
		ENEMY_START_X(700),
		ENEMY_START_Y(50),
		backgroundTextureID(0),
		stateTimer(0),
		waveMessageTimer(0),
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
		}
		else {
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
	Monster enemyTemplate;
	std::vector<Monster> activeEnemies;
	CombatState currentState;
	int currentWave;
	float stateTimer;
	float waveMessageTimer;
	float attackFeedbackTimer;
	std::string currentAttackName;
	float currentAttackDamage;
	bool playerHit;
	unsigned int backgroundTextureID; // Cache background
	bool isTower3;
	float bgScrollOffset;
	int enemiesRemainingToSpawn;
	float nextEnemySpawnTimer;

	const int TOTAL_WAVES;
	const float PLAYER_START_X;
	const float PLAYER_START_Y;
	const float ENEMY_START_X;
	const float ENEMY_START_Y;
};

#endif
