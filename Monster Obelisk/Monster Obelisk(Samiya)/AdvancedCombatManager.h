#ifndef ADVANCEDCOMBATMANAGER_H
#define ADVANCEDCOMBATMANAGER_H

#include <cmath>
#include <vector>
#include <string>
#include <cstdio>
#include "CombatManager.h"

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

		bosses.clear();
		playerWalkFrames.clear();
		playerIdleFrames.clear();
		playerLBFrames.clear();
		playerTCFrames.clear();
		enemyWalkFrames.clear();
		enemyAttack1Frames.clear();
		enemyAttack2Frames.clear();

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

		// Player animations
		if (CombatManager::GetInstance().dawnUnlocked) {
			CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(playerWalkFrames, "Image//Monstrum Obelisk monster images//Player Monsters//Dawn//Dawn Walk right", 25);
			CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(playerIdleFrames, "Image//Monstrum Obelisk monster images//Player Monsters//Dawn//Dawn Walk right", 25);
			CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(playerLBFrames,   "Image//Monstrum Obelisk monster images//Player Monsters//Dawn//Dawn Attack right", 25);
			CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(playerTCFrames,   "Image//Monstrum Obelisk monster images//Player Monsters//Dawn//Dawn Special Attack right", 25);
		} else {
			LoadFrames(playerWalkFrames, "Image//myMonster//myMonster_walk",     25);
			LoadFrames(playerIdleFrames, "Image//myMonster//myMonster_standing", 16);
			LoadFrames(playerLBFrames,   "Image//myMonster//myMonser_lb",        25); // Lightning Blast
			LoadFrames(playerTCFrames,   "Image//myMonster//myMonster_tc",       25); // Thunder Crash
		}

		// Remove old generic loader
		// LoadFrames(enemyFrames, "Image//black//black_running", 16);

		playerX = 100.0f;
		SetupWave();
	}

	void SetupWave() {
		// Scale player stats per wave (Base stats increased for Tower 2)
		float dawnBonus = CombatManager::GetInstance().dawnUnlocked ? 15.0f : 0.0f;
		playerMaxHP       = 150.0f + (50.0f * currentWave);
		playerHP          = playerMaxHP;
		playerAttackPower = 60.0f + dawnBonus + (20.0f * currentWave);

		if (currentWave == 0) enemiesRemainingInWave = 1;
		else if (currentWave == 1) enemiesRemainingInWave = 2;
		else enemiesRemainingInWave = 3;

		std::string folderName = "Husk";
		std::string walkFolderName = "Husk Walk left";
		if (currentWave == 1) {
			folderName = "Jellum";
			walkFolderName = "Jellum Walking left";
		} else if (currentWave == 2) {
			folderName = "Logum";
			walkFolderName = "Logum Walking left";
		}

		std::string prefixWalk = "Image//Monstrum Obelisk monster images//" + folderName + "//" + walkFolderName;
		std::string prefixAtk1 = "Image//Monstrum Obelisk monster images//" + folderName + "//" + folderName + " Attack left";
		std::string prefixAtk2 = "Image//Monstrum Obelisk monster images//" + folderName + "//" + folderName + " Special Attack left";
		
		CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(enemyWalkFrames, prefixWalk, 25);
		CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(enemyAttack1Frames, prefixAtk1, 25);
		CombatManager::GetInstance().LoadAnimationFramesZeroIndexed(enemyAttack2Frames, prefixAtk2, 25);

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
		if (animTimer >= 0.10f) {
			animTimer = 0;
			playerFrame++;
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
				} else {
					damageToDeal *= 0.6f;
					activeEnemies[i].currentAttackType = 1;
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

			std::vector<unsigned int>* anim = &playerIdleFrames;
			if (isPlayerMoving || isTransitioning) {
				anim = &playerWalkFrames;
			}

			if (currentState == ADV_PLAYER_ATTACK) {
				if (!CombatManager::GetInstance().dawnUnlocked) {
					drawW = 150; drawH = 150;
					drawX -= 25; drawY -= 25;
				}
				anim = (currentAttackType == 2) ? &playerTCFrames : &playerLBFrames;
			}
			if (!anim->empty()) {
				unsigned int tex = (*anim)[playerFrame % anim->size()];
				iShowImage(drawX, drawY, drawW, drawH, tex);
			}
		}

		// Player HP bar
		DrawHealthBar(playerX + 20, 180, playerHP, playerMaxHP, 0, 255, 0);
		char phpBuf[32];
		sprintf_s(phpBuf, 32, "HP: %d/%d", (int)playerHP, (int)playerMaxHP);
		iSetColor(255, 255, 255);
		iText(playerX + 20, 200, phpBuf, (void*)0x0008);

		// Draw Enemy (animated)
		if (!isTransitioning) {
			for (size_t i = 0; i < activeEnemies.size(); i++) {
				int ew = 100, eh = 100;
				const auto& e = activeEnemies[i];

				std::vector<unsigned int>* currentAnim = &enemyWalkFrames;
				if (e.currentAttackType == 1 && !enemyAttack1Frames.empty()) {
					currentAnim = &enemyAttack1Frames;
				} else if (e.currentAttackType == 2 && !enemyAttack2Frames.empty()) {
					currentAnim = &enemyAttack2Frames;
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
		iText(220, 535, "A/D: Move   W: Attack   F: Thunder Crash   ESC: Exit", (void*)0x0008);

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
	std::vector<unsigned int> enemyWalkFrames;
	std::vector<unsigned int> enemyAttack1Frames;
	std::vector<unsigned int> enemyAttack2Frames;

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
		std::string name;
		ActiveEnemy() : x(0), y(0), hp(100), maxHP(100), attackTimer(0), frame(0), animTimer(0), currentAttackType(0), attacksPerformed(0), name("") {}
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