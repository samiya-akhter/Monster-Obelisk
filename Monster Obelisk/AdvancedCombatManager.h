#ifndef ADVANCEDCOMBATMANAGER_H
#define ADVANCEDCOMBATMANAGER_H

#include <cmath>
#include <vector>
#include <string>
#include <cstdio>
#include "CombatManager.h"
#include "audio.h"
// Forward declarations
extern void iShowImage(int x, int y, int width, int height, unsigned int texture);
extern void iSetColor(double r, double g, double b);
extern void iFilledRectangle(double left, double bottom, double dx, double dy);
extern void iRectangle(double left, double bottom, double dx, double dy);
extern void iText(double x, double y, const char* str, void* font);
extern unsigned int iLoadImage(const char* filename, int rIgnore, int gIgnore, int bIgnore);
extern unsigned int iLoadImage(const char* filename);

struct BossData {
	std::string name;
	std::string texturePath;
	std::string bgPath;
	float health;
	float attack;
	unsigned int bgID;
	unsigned int textureID;
};

enum AdvCombatState {
	ADV_PLAYER_TURN,
	ADV_PLAYER_ATTACK,
	ADV_VICTORY
};

class AdvancedCombatManager {
public:
	static AdvancedCombatManager& GetInstance() {
		static AdvancedCombatManager instance;
		return instance;
	}

	void HealPlayer(float percentage) {
		playerHP += playerMaxHP * percentage;
		if (playerHP > playerMaxHP) playerHP = playerMaxHP;
	}

	void Init() {
		CombatManager::GetInstance().lastTowerPlayed = 2;
		currentWave         = 0;
		isTransitioning     = false;
		bgOffset            = 0;
		playerFrame         = 0;
		animTimer           = 0;
		stateTimer          = 0;
		attackFeedbackTimer = 0;
		currentAttackDamage = 0;
		currentAttackType   = 0;
		currentAttackName   = "";
		playerHit           = false;
		currentState        = ADV_PLAYER_TURN;
		isPlayerMoving      = false;

		CombatManager::GetInstance().damagePotionUsed = false;

		bosses.clear();
		playerWalkFrames.clear();
		playerIdleFrames.clear();
		playerLBFrames.clear();
		playerTCFrames.clear();
		playerWalkFramesL.clear();
		playerIdleFramesL.clear();
		playerLBFramesL.clear();
		playerTCFramesL.clear();
		enemyWalkFrames.clear();
		enemyAttack1Frames.clear();
		enemyAttack2Frames.clear();
		enemyWalkFramesR.clear();
		enemyAttack1FramesR.clear();
		enemyAttack2FramesR.clear();

		// Boss 1: Forest
		bosses.push_back({ "Shadow Golem",  "Image//boss1.png", "Image//combat render.png",  180.0f, 8.0f, 0, 0 });
		// Boss 2: Volcano
		bosses.push_back({ "Inferno Drake", "Image//boss2.png", "Image//combat render.png", 280.0f, 12.0f, 0, 0 });
		// Boss 3: Void
		bosses.push_back({ "Void Sentinel", "Image//boss3.png", "Image//combat render.png",  450.0f, 18.0f, 0, 0 });

		for (auto& b : bosses) {
			b.bgID      = iLoadImage(b.bgPath.c_str());
			b.textureID = iLoadImage(b.texturePath.c_str(), 255, 255, 255);
		}

		// Player animations (right-facing default, left-facing alt)
		if (CombatManager::GetInstance().dawnUnlocked) {
			CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(playerWalkFrames,  "Image//Monster Images//Player Monsters//Dawn//Dawn Walk right",           25);
			CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(playerIdleFrames,  "Image//Monster Images//Player Monsters//Dawn//Dawn Walk right",           25);
			CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(playerLBFrames,    "Image//Monster Images//Player Monsters//Dawn//Dawn Attack right",         25);
			CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(playerTCFrames,    "Image//Monster Images//Player Monsters//Dawn//Dawn Special Attack right",  25);
			CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(playerWalkFramesL, "Image//Monster Images//Player Monsters//Dawn//Dawn Walk left",            25);
			CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(playerIdleFramesL, "Image//Monster Images//Player Monsters//Dawn//Dawn Walk left",            25);
			CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(playerLBFramesL,   "Image//Monster Images//Player Monsters//Dawn//Dawn Attack left",          25);
			CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(playerTCFramesL,   "Image//Monster Images//Player Monsters//Dawn//Dawn Special Attack left",   25);
		} else {
			LoadFrames(playerWalkFrames,  "Image//Monster Images//Player Monsters//Vivi//Vivi Walk Right//vivi_walk",            25);
			LoadFrames(playerIdleFrames,  "Image//Monster Images//Player Monsters//Vivi//Vivi Idle Right//vivo_standing",        16);
			LoadFrames(playerLBFrames,    "Image//Monster Images//Player Monsters//Vivi//Vivi Attack Right//vivi_lb",            25);
			LoadFrames(playerTCFrames,    "Image//Monster Images//Player Monsters//Vivi//Vivi Special Attack Right//vivi_tc",    25);
			LoadFrames(playerWalkFramesL, "Image//Monster Images//Player Monsters//Vivi//Vivi Walk Left//vivi_walk",             25);
			LoadFrames(playerIdleFramesL, "Image//Monster Images//Player Monsters//Vivi//Vivi Idle Left//vivo_standing",         16);
			LoadFrames(playerLBFramesL,   "Image//Monster Images//Player Monsters//Vivi//Vivi Attack Left//vivi_lb",             25);
			LoadFrames(playerTCFramesL,   "Image//Monster Images//Player Monsters//Vivi//Vivi Special Attack Left//vivi_tc",     25);
		}
		playerFacingRight = true;

		playerX = 100.0f;
		SetupWave();
	}

	void SetupWave() {
		// Scale player stats per wave (Base stats increased for Tower 2)
		float dawnBonus = CombatManager::GetInstance().dawnUnlocked ? 15.0f : 0.0f;
		float drakeBonus = CombatManager::GetInstance().drakeUnlocked ? 35.0f : 0.0f;
		float totalBonus = (drakeBonus > 0) ? drakeBonus : dawnBonus;

		playerMaxHP       = 150.0f + (50.0f * currentWave) + (totalBonus * 2.0f);
		playerHP          = playerMaxHP;
		playerAttackPower = 60.0f + totalBonus + (20.0f * currentWave);

		if (currentWave == 0) enemiesRemainingInWave = 1;
		else if (currentWave == 1) enemiesRemainingInWave = 2;
		else enemiesRemainingInWave = 3;

		std::string folderName = "Husk";
		std::string walkFolderName    = "Husk Walk left";
		std::string walkFolderNameR   = "Husk Walk right";
		if (currentWave == 1) {
			folderName        = "Jellum";
			walkFolderName    = "Jellum Walking left";
			walkFolderNameR   = "Jellum Walk right";
		} else if (currentWave == 2) {
			folderName        = "Logum";
			walkFolderName    = "Logum Walking left";
			walkFolderNameR   = "Logum Walk right";
		}

		std::string prefixWalk  = "Image//Monster Images//" + folderName + "//" + walkFolderName;
		std::string prefixAtk1  = "Image//Monster Images//" + folderName + "//" + folderName + " Attack left";
		std::string prefixAtk2  = "Image//Monster Images//" + folderName + "//" + folderName + " Special Attack left";
		std::string prefixWalkR = "Image//Monster Images//" + folderName + "//" + walkFolderNameR;
		std::string prefixAtk1R = "Image//Monster Images//" + folderName + "//" + folderName + " Attack right";
		std::string prefixAtk2R = "Image//Monster Images//" + folderName + "//" + folderName + " Special Attack right";

		CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(enemyWalkFrames,    prefixWalk,  25);
		CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(enemyAttack1Frames, prefixAtk1,  25);
		CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(enemyAttack2Frames, prefixAtk2,  25);
		CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(enemyWalkFramesR,   prefixWalkR, 25);
		CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(enemyAttack1FramesR, prefixAtk1R, 25);
		CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(enemyAttack2FramesR, prefixAtk2R, 25);

		activeEnemies.clear();
		int idx = (currentWave < 3) ? currentWave : 2;
		ActiveEnemy newEnemy;
		newEnemy.x = 1000.0f;
		newEnemy.y = 50.0f;
		newEnemy.hp = bosses[idx].health;
		newEnemy.maxHP = bosses[idx].health;
		newEnemy.attackTimer = 0.0f;
		newEnemy.name = folderName;
		activeEnemies.push_back(newEnemy);
		enemiesRemainingInWave--;
		nextSpawnTimer = 5.0f; // 5 seconds before next enemy spawns

		isTransitioning     = false;
		currentState        = ADV_PLAYER_TURN;
		stateTimer          = 0;
		attackFeedbackTimer = 0;
		waveMessageTimer    = 3.0f;
	}

	void Update(float deltaTime) {
		if (attackFeedbackTimer > 0) attackFeedbackTimer -= deltaTime;
		if (waveMessageTimer    > 0) waveMessageTimer    -= deltaTime;
		stateTimer += deltaTime;

		if (currentState == ADV_VICTORY) return;

		// Wave transition countdown
		if (isTransitioning) {
			if (currentWave == 0 || currentWave == 1) {
				bgOffset -= 300.0f * deltaTime;
			}
			
			animTimer += deltaTime;
			if (animTimer >= 0.10f) {
				animTimer = 0;
				playerFrame++;
			}

			transitionTimer -= deltaTime;
			if (transitionTimer <= 0) {
				currentWave++;
				if (currentWave < 3) {
					SetupWave();
				} else {
					currentState = ADV_VICTORY;
					isTransitioning = false;
				}
			}
			return;
		}

		// Animation cycling
		animTimer += deltaTime;
		if (animTimer >= 0.12f) {
			animTimer = 0;
			// During attack, only advance if we haven't passed the full animation
			if (currentState == ADV_PLAYER_ATTACK) {
				if (!playerLBFrames.empty() && currentAttackType == 1) {
					if (playerFrame < (int)playerLBFrames.size() - 1)
						playerFrame++;
				} else if (!playerTCFrames.empty() && currentAttackType == 2) {
					if (playerFrame < (int)playerTCFrames.size() - 1)
						playerFrame++;
				}
			} else {
				playerFrame++;
			}
			isPlayerMoving = false;
		}

		// Enemy Animations
		for (size_t i = 0; i < activeEnemies.size(); i++) {
			activeEnemies[i].animTimer += deltaTime;
			if (activeEnemies[i].animTimer >= 0.05f) { 
				activeEnemies[i].animTimer = 0;
				activeEnemies[i].frame++;

				if (activeEnemies[i].currentAttackType == 1 && !enemyAttack1Frames.empty()) {
					if (activeEnemies[i].frame >= (int)enemyAttack1Frames.size()) {
						activeEnemies[i].currentAttackType = 0;
						activeEnemies[i].frame = 0;
					}
				} else if (activeEnemies[i].currentAttackType == 2 && !enemyAttack2Frames.empty()) {
					if (activeEnemies[i].frame >= (int)enemyAttack2Frames.size()) {
						activeEnemies[i].currentAttackType = 0;
						activeEnemies[i].frame = 0;
					}
				} else {
					if (!enemyWalkFrames.empty() && activeEnemies[i].frame >= (int)enemyWalkFrames.size()) {
						activeEnemies[i].frame = 0;
					}
				}
			}
		}

		// Handle staggered spawning
		if (enemiesRemainingInWave > 0) {
			nextSpawnTimer -= deltaTime;
			if (nextSpawnTimer <= 0) {
				int idx = (currentWave < 3) ? currentWave : 2;
				ActiveEnemy newEnemy;
				newEnemy.x = 1000.0f;
				newEnemy.y = 50.0f;
				newEnemy.hp = bosses[idx].health;
				newEnemy.maxHP = bosses[idx].health;
				newEnemy.attackTimer = 0.0f;
				
				std::string spawnFolderName = "Husk";
				if (currentWave == 1) spawnFolderName = "Jellum";
				else if (currentWave == 2) spawnFolderName = "Logum";
				newEnemy.name = spawnFolderName;

				activeEnemies.push_back(newEnemy);
				enemiesRemainingInWave--;
				nextSpawnTimer = 5.0f;
			}
		}

		// Enemy AI: move towards player
		for (size_t i = 0; i < activeEnemies.size(); i++) {
			float dist = fabs(playerX - activeEnemies[i].x);
			float stopDist = 150.0f + (i * 60.0f);
			// Update enemy facing direction based on player
			activeEnemies[i].facingRight = (playerX > activeEnemies[i].x);

			if (dist > stopDist) {
				float speed = 120.0f;
				if (playerX > activeEnemies[i].x) activeEnemies[i].x += speed * deltaTime;
				else                              activeEnemies[i].x -= speed * deltaTime;
			}

			// Enemy attacks
			activeEnemies[i].attackTimer -= deltaTime;
			if (dist < stopDist && activeEnemies[i].attackTimer <= 0) {
				activeEnemies[i].attacksPerformed++;
				int idx = (currentWave < 3) ? currentWave : 2;
				float damageToDeal = bosses[idx].attack;

				if (activeEnemies[i].attacksPerformed % 4 == 0) {
					damageToDeal *= 2.5f;
					activeEnemies[i].currentAttackType = 2;
					AudioSystem::PlaySFX(activeEnemies[i].name, 2);
				} else {
					damageToDeal *= 0.6f;
					activeEnemies[i].currentAttackType = 1;
					AudioSystem::PlaySFX(activeEnemies[i].name, 1);
				}

				activeEnemies[i].frame = 0;
				playerHP -= damageToDeal;
				activeEnemies[i].attackTimer = 1.5f;

				if (playerHP <= 0) {
					playerHP = 0;
					CombatManager::GetInstance().lives--;
					if (CombatManager::GetInstance().lives <= 0) {
						extern int gameState;
						gameState = 5;
						return;
					}
					SetupWave();
					return;
				}
			}
		}

		// Process player attack
		if (currentState == ADV_PLAYER_ATTACK && stateTimer > 0.25f) {
			bool hitSomething = false;
			for (int i = (int)activeEnemies.size() - 1; i >= 0; i--) {
				float attackDist = fabs(playerX - activeEnemies[i].x);
				if (attackDist < 250.0f) {
					activeEnemies[i].hp -= currentAttackDamage;
					hitSomething = true;
					if (activeEnemies[i].hp <= 0) {
						activeEnemies.erase(activeEnemies.begin() + i);
					}
				}
			}
			playerHit = hitSomething;

			if (activeEnemies.empty() && enemiesRemainingInWave <= 0) {
				StartTransition();
				return;
			}
			currentState = ADV_PLAYER_TURN;
			stateTimer = 0;
		}
	}

	void PlayerAttack(int type) {
		if (currentState != ADV_PLAYER_TURN) return;

		currentAttackType = type;
		playerFrame       = 0;

		if (type == 1) {
			currentAttackName   = "LIGHTNING BLAST!";
			currentAttackDamage = playerAttackPower;
		} else if (type == 2) {
			if (currentWave < 2) return;
			currentAttackName   = "THUNDER CRASH!";
			currentAttackDamage = playerAttackPower * 2.0f;
		}

		if (CombatManager::GetInstance().damagePotionUsed) {
			currentAttackDamage *= 1.5f;
		}

		std::string currentPlayerName = "Vivi";
		if (CombatManager::GetInstance().dawnUnlocked) currentPlayerName = "Dawn";
		// AdvancedCombatManager doesn't seem to load Drake, but if it did it would be here
		
		AudioSystem::PlaySFX(currentPlayerName, type);

		currentState        = ADV_PLAYER_ATTACK;
		stateTimer          = 0;
		attackFeedbackTimer = 1.0f;
	}

	void PlayerAttack() { PlayerAttack(1); }

	bool IsVictory() const { return currentState == ADV_VICTORY; }

	void StartTransition() {
		activeEnemies.clear();
		isTransitioning = true;
		transitionTimer = 2.5f;
		currentAttackType = 0; // Reset so walk anim renders, not attack
		currentState = ADV_PLAYER_TURN; // Allow walk frames to render
		playerFrame = 0;
	}

	void Render() {
		int idx = (currentWave < 3) ? currentWave : 2;

		// Background
		float wrapOffset = fmod(bgOffset, 1100.0f);
		if (wrapOffset > 0) wrapOffset -= 1100.0f;
		iShowImage((int)wrapOffset, 0, 1100, 600, bosses[idx].bgID);
		iShowImage((int)wrapOffset + 1100, 0, 1100, 600, bosses[idx].bgID);

		// Draw Player
		{
			int drawW = 100, drawH = 100;
			int drawX = (int)playerX;
			int drawY = 50;

			// Pick direction-appropriate frame sets
			std::vector<unsigned int>* playerWalk = playerFacingRight ? &playerWalkFrames : &playerWalkFramesL;
			std::vector<unsigned int>* playerIdle = playerFacingRight ? &playerIdleFrames : &playerIdleFramesL;
			std::vector<unsigned int>* playerLB   = playerFacingRight ? &playerLBFrames   : &playerLBFramesL;
			std::vector<unsigned int>* playerTC   = playerFacingRight ? &playerTCFrames   : &playerTCFramesL;

			std::vector<unsigned int>* anim = playerIdle;
			if (isPlayerMoving || isTransitioning) {
				anim = playerWalk;
			}

			// Only show attack animation when actively in attack state AND attack type is set
			if (currentState == ADV_PLAYER_ATTACK && currentAttackType != 0 && !isTransitioning) {
				if (!CombatManager::GetInstance().dawnUnlocked) {
					drawW = 150; drawH = 150;
					drawX -= 25; drawY -= 25;
				}
				anim = (currentAttackType == 2) ? playerTC : playerLB;
			}
			if (!anim->empty()) {
				unsigned int tex = (*anim)[playerFrame % anim->size()];
				iShowImage(drawX, drawY, drawW, drawH, tex);
			}
		} // <-- Missed closing brace for Draw Player scope

		// Player HP bar
		DrawHealthBar(playerX + 20, 180, playerHP, playerMaxHP, 0, 255, 0);
		char hud[128];
		sprintf_s(hud, sizeof(hud), "Wave: %d/3   HP: %d/%d   PWR: %d", currentWave + 1, (int)playerHP, (int)playerMaxHP, (int)playerAttackPower);
		iSetColor(255, 255, 255);
		iText(20, 575, hud, (void*)0x0006);

		sprintf_s(hud, sizeof(hud), "H: Heal (%d)   X: Damage (%d)", CombatManager::GetInstance().healPotionCount, CombatManager::GetInstance().damagePotionCount);
		iSetColor(200, 200, 255);
		iText(20, 555, hud, (void*)0x0008);

		// Draw Enemy (animated) — direction-aware
		if (!isTransitioning) {
			for (size_t i = 0; i < activeEnemies.size(); i++) {
				int ew = 100, eh = 100;
				const auto& e = activeEnemies[i];

				// Pick left-facing (default) or right-facing based on enemy direction
				std::vector<unsigned int>* eWalk = e.facingRight ? &enemyWalkFramesR    : &enemyWalkFrames;
				std::vector<unsigned int>* eAtk1 = e.facingRight ? &enemyAttack1FramesR : &enemyAttack1Frames;
				std::vector<unsigned int>* eAtk2 = e.facingRight ? &enemyAttack2FramesR : &enemyAttack2Frames;

				std::vector<unsigned int>* currentAnim = eWalk;
				if (e.currentAttackType == 1 && !eAtk1->empty()) {
					currentAnim = eAtk1;
				} else if (e.currentAttackType == 2 && !eAtk2->empty()) {
					currentAnim = eAtk2;
				}

				if (!currentAnim->empty()) {
					unsigned int eTex = (*currentAnim)[e.frame % currentAnim->size()];
					iShowImage((int)e.x, (int)e.y, ew, eh, eTex);
				}

				DrawHealthBar(e.x, e.y + 110, e.hp, e.maxHP, 255, 0, 0);
				char ehpBuf[64];
				sprintf_s(ehpBuf, 64, "%s HP: %d/%d", e.name.c_str(), (int)e.hp, (int)e.maxHP);
				iSetColor(255, 255, 255);
				iText(e.x - 10, e.y + 125, ehpBuf, (void*)0x0008);
			}
		}

		// Attack feedback
		if (attackFeedbackTimer > 0) {
			iSetColor(255, 50, 50);
			iText(playerX, 300, currentAttackName.c_str(), (void*)0x0008);
		}

		// Wave start message
		if (waveMessageTimer > 0) {
			iSetColor(255, 215, 0);
			char waveMsg[64];
			sprintf_s(waveMsg, 64, "WAVE %d: %s", currentWave + 1, bosses[idx].name.c_str());
			iText(250, 450, waveMsg, (void*)0x0005);
		}

		// Wave cleared message
		if (isTransitioning) {
			iSetColor(0, 255, 0);
			char buf[32];
			sprintf_s(buf, 32, "WAVE %d CLEARED!", currentWave + 1);
			iText(350, 300, buf, (void*)0x0005);
		}

		// Wave counter
		char waveBuf[32];
		int displayWave = currentWave + 1;
		if (displayWave > 3) displayWave = 3;
		sprintf_s(waveBuf, 32, "Wave: %d / 3", displayWave);
		iSetColor(255, 255, 255);
		iText(430, 555, waveBuf, (void*)0x0008);

		// Controls hint
		iSetColor(180, 180, 180);
		iText(160, 580, "A/D: Move   W: Attack   F: Thunder Crash (Wave 3)   ESC: Exit", (void*)0x0008);

		// Victory overlay
		if (currentState == ADV_VICTORY) {
			iSetColor(255, 215, 0);
			iText(400, 300, "VICTORY!", (void*)0x0006);
			iSetColor(255, 255, 255);
			iText(320, 260, "Press ESC to return to map", (void*)0x0005);
		}
	}

	void MovePlayer(float dx) {
		if (currentState == ADV_PLAYER_TURN || currentState == ADV_PLAYER_ATTACK || isTransitioning) {
			if (dx > 0) playerFacingRight = true;
			else if (dx < 0) playerFacingRight = false;
			playerX += dx;
			isPlayerMoving = true;
			if (isTransitioning && (currentWave == 0 || currentWave == 1)) {
				bgOffset -= dx * 2.2f;
			} else {
				bgOffset -= dx * 0.4f;
			}
			if (playerX < 0)   playerX = 0;
			if (playerX > 800) playerX = 800;
		}
	}

private:
	void LoadFrames(std::vector<unsigned int>& frames, const std::string& prefix, int count) {
		frames.clear();
		char path[256];
		for (int i = 1; i <= count; i++) {
			sprintf_s(path, 256, "%s (%d).png", prefix.c_str(), i);
			frames.push_back(iLoadImage(path, 255, 255, 255));
		}
	}

	void DrawHealthBar(float x, float y, float current, float maxVal, float r, float g, float b) {
		iSetColor(100, 100, 100);
		iFilledRectangle(x, y, 100, 10);
		if (maxVal > 0) {
			float ratio = current / maxVal;
			if (ratio < 0) ratio = 0;
			if (ratio > 1) ratio = 1;
			iSetColor(r, g, b);
			iFilledRectangle(x, y, 100 * ratio, 10);
		}
		iSetColor(255, 255, 255);
		iRectangle(x, y, 100, 10);
	}

	std::vector<BossData>     bosses;
	std::vector<unsigned int> playerWalkFrames;
	std::vector<unsigned int> playerIdleFrames;
	std::vector<unsigned int> playerLBFrames;
	std::vector<unsigned int> playerTCFrames;
	std::vector<unsigned int> playerWalkFramesL;   // left-facing
	std::vector<unsigned int> playerIdleFramesL;
	std::vector<unsigned int> playerLBFramesL;
	std::vector<unsigned int> playerTCFramesL;
	std::vector<unsigned int> enemyWalkFrames;      // left-facing (default)
	std::vector<unsigned int> enemyAttack1Frames;
	std::vector<unsigned int> enemyAttack2Frames;
	std::vector<unsigned int> enemyWalkFramesR;     // right-facing (when player is to enemy's right)
	std::vector<unsigned int> enemyAttack1FramesR;
	std::vector<unsigned int> enemyAttack2FramesR;

	bool playerFacingRight;

	AdvCombatState currentState;
	int            currentWave;
	int            currentAttackType;
	std::string    currentAttackName;
	float          currentAttackDamage;
	float          playerAttackPower;
	bool           playerHit;

	int   playerFrame;
	float animTimer;

	struct ActiveEnemy {
		float x, y;
		float hp, maxHP;
		float attackTimer;
		int frame;
		float animTimer;
		int currentAttackType; // 0=walk, 1=attack1, 2=attack2
		int attacksPerformed;
		bool facingRight;      // true = enemy faces right (player is to their right)
		std::string name;
		ActiveEnemy() : x(0), y(0), hp(100), maxHP(100), attackTimer(0), frame(0), animTimer(0), currentAttackType(0), attacksPerformed(0), facingRight(false), name("") {}
	};
	std::vector<ActiveEnemy> activeEnemies;
	float nextSpawnTimer;

	float playerX;
	float playerHP, playerMaxHP;
	float bgOffset;
	float transitionTimer, stateTimer, waveMessageTimer;
	float attackFeedbackTimer;
	bool  isTransitioning;
	bool  isPlayerMoving;
	int   enemiesRemainingInWave;
};

#endif