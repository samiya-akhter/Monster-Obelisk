#ifndef ADVANCEDCOMBATMANAGER_H
#define ADVANCEDCOMBATMANAGER_H

#include <cmath>
#include <vector>
#include <string>
#include <cstdio>

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
		currentWave         = 0;
		isTransitioning     = false;
		bgOffset            = 0;
		playerFrame         = 0;
		enemyFrame          = 0;
		animTimer           = 0;
		stateTimer          = 0;
		enemyAttackTimer    = 0;
		attackFeedbackTimer = 0;
		currentAttackDamage = 0;
		currentAttackType   = 0;
		currentAttackName   = "";
		playerHit           = false;
		currentState        = ADV_PLAYER_TURN;

		bosses.clear();
		playerWalkFrames.clear();
		playerIdleFrames.clear();
		playerLBFrames.clear();
		playerTCFrames.clear();
		enemyFrames.clear();

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

		// Player animations (myMonster folder)
		LoadFrames(playerWalkFrames, "Image//myMonster//myMonster_walk",     25);
		LoadFrames(playerIdleFrames, "Image//myMonster//myMonster_standing", 16);
		LoadFrames(playerLBFrames,   "Image//myMonster//myMonser_lb",        25); // Lightning Blast
		LoadFrames(playerTCFrames,   "Image//myMonster//myMonster_tc",       25); // Thunder Crash

		// Enemy animations
		LoadFrames(enemyFrames, "Image//black//black_running", 16);

		playerX = 100.0f;
		SetupWave();
	}

	void SetupWave() {
		// Scale player stats per wave
		playerMaxHP       = 100.0f + (50.0f * currentWave);
		playerHP          = playerMaxHP;
		playerAttackPower = 40.0f + (20.0f * currentWave);

		int idx  = (currentWave < 3) ? currentWave : 2;
		enemyHP  = bosses[idx].health;
		
		// Reset positions
		playerX  = 100.0f;
		enemyX   = 700.0f;

		isTransitioning     = false;
		currentState        = ADV_PLAYER_TURN;
		stateTimer          = 0;
		enemyAttackTimer    = 0;
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
			// Player moves manually now during this state!
			
			// Keep animations playing if they are moving (relies on frame update)
			animTimer += deltaTime;
			if (animTimer >= 0.25f) {
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
				}
			}
			return;
		}

		// Animation cycling
		animTimer += deltaTime;
		if (animTimer >= 0.25f) {
			animTimer = 0;
			playerFrame++;
			enemyFrame++;
		}

		// Enemy AI: move towards player (slowed down to 70 px/s)
		float dist = fabs(playerX - enemyX);
		const float stopDist = 150.0f;
		if (dist > stopDist) {
			float speed = 30.0f;
			if (playerX > enemyX) enemyX += speed * deltaTime;
			else                   enemyX -= speed * deltaTime;
		}

		// Enemy attacks with cooldown (1 attack per 1.5 s)
		enemyAttackTimer -= deltaTime;
		if (dist < stopDist && enemyAttackTimer <= 0) {
			int idx   = (currentWave < 3) ? currentWave : 2;
			playerHP -= bosses[idx].attack;
			enemyAttackTimer = 1.5f;

			// If player loses: restart this wave immediately
			if (playerHP <= 0) {
				playerHP = 0;
				SetupWave();   // Replay the level
				return;
			}
		}

		// Process player attack after 1.0 s (AABB distance check)
		if (currentState == ADV_PLAYER_ATTACK && stateTimer > 1.0f) {
			float attackDist = fabs(playerX - enemyX);
			if (attackDist < 250.0f) {
				enemyHP  -= currentAttackDamage;
				playerHit = true;
				if (enemyHP <= 0) {
					enemyHP = 0;
					StartTransition();
					return;
				}
			} else {
				playerHit = false; // Miss
			}
			currentState = ADV_PLAYER_TURN;
			stateTimer   = 0;
		}
	}

	// type 1 = Lightning Blast (key W), type 2 = Thunder Crash (key F, wave 3+)
	void PlayerAttack(int type) {
		if (currentState != ADV_PLAYER_TURN) return;

		currentAttackType = type;
		playerFrame       = 0;

		if (type == 1) {
			currentAttackName   = "LIGHTNING BLAST!";
			currentAttackDamage = playerAttackPower;
		} else if (type == 2) {
			if (currentWave < 2) return; // Unlocked at wave 3 only
			currentAttackName   = "THUNDER CRASH!";
			currentAttackDamage = playerAttackPower * 2.0f;
		}

		currentState        = ADV_PLAYER_ATTACK;
		stateTimer          = 0;
		attackFeedbackTimer = 1.0f;
	}

	// No-arg version for compatibility
	void PlayerAttack() { PlayerAttack(1); }

	void StartTransition() {
		enemyHP         = 0;
		isTransitioning = true;
		transitionTimer = 5.0f; // Give more time to manually walk forward
	}

	void Render() {
		int idx = (currentWave < 3) ? currentWave : 2;

		// Background
		float wrapOffset = fmod(bgOffset, 1100.0f);
		if (wrapOffset > 0) wrapOffset -= 1100.0f;
		iShowImage((int)wrapOffset, 0, 1100, 600, bosses[idx].bgID);
		iShowImage((int)wrapOffset + 1100, 0, 1100, 600, bosses[idx].bgID);

		// Draw Player (animated)
		{
			int drawW = 100, drawH = 100;
			std::vector<unsigned int>* anim = &playerWalkFrames;
			if (currentState == ADV_PLAYER_ATTACK) {
				drawW = 150; drawH = 150;
				anim = (currentAttackType == 2) ? &playerTCFrames : &playerLBFrames;
			}
			if (!anim->empty()) {
				unsigned int tex = (*anim)[playerFrame % anim->size()];
				iShowImage((int)playerX, 50, drawW, drawH, tex);
			}
		}

		// Player HP bar
		DrawHealthBar(playerX + 20, 180, playerHP, playerMaxHP, 0, 255, 0);
		char phpBuf[32];
		sprintf_s(phpBuf, 32, "HP: %d/%d", (int)playerHP, (int)playerMaxHP);
		iSetColor(255, 255, 255);
		iText(playerX + 20, 200, phpBuf, (void*)0x0008);

		// Draw Enemy (animated)
		if (!enemyFrames.empty()) {
			unsigned int tex = enemyFrames[enemyFrame % enemyFrames.size()];
			iShowImage((int)enemyX, 50, 100, 100, tex);
		} else {
			iShowImage((int)enemyX, 50, 100, 100, bosses[idx].textureID);
		}

		// Enemy HP bar
		DrawHealthBar(enemyX + 20, 180, enemyHP, bosses[idx].health, 255, 0, 0);
		char ehpBuf[32];
		sprintf_s(ehpBuf, 32, "HP: %d/%d", (int)enemyHP, (int)bosses[idx].health);
		iSetColor(255, 255, 255);
		iText(enemyX + 20, 200, ehpBuf, (void*)0x0008);

		// Boss name
		iSetColor(255, 100, 100);
		iText(enemyX + 20, 220, bosses[idx].name.c_str(), (void*)0x0008);

		// Attack feedback
		if (attackFeedbackTimer > 0) {
			iSetColor(255, 50, 50);
			iText(playerX, 300, currentAttackName.c_str(), (void*)0x0008);
		}

		// Wave start message
		if (waveMessageTimer > 0) {
			iSetColor(255, 215, 0);
			char waveMsg[64];
			if (currentWave == 2) {
				sprintf_s(waveMsg, 64, "WAVE 3: THUNDER CRASH UNLOCKED! (Press F)");
			} else {
				sprintf_s(waveMsg, 64, "WAVE %d: %s", currentWave + 1, bosses[idx].name.c_str());
			}
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
		sprintf_s(waveBuf, 32, "Wave: %d / 3", currentWave + 1);
		iSetColor(255, 255, 255);
		iText(430, 555, waveBuf, (void*)0x0008);

		// Controls hint
		iSetColor(180, 180, 180);
		iText(220, 535, "A/D: Move   W: Attack   F: Thunder Crash (Wave 3)   ESC: Exit", (void*)0x0008);

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
			if (isTransitioning) {
				bgOffset -= dx * 1.5f; // Fast Parallax effect to simulate running to next wave
			} else {
				bgOffset -= dx * 0.1f; // Subtle background parallax during normal combat
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
	std::vector<unsigned int> enemyFrames;

	AdvCombatState currentState;
	int            currentWave;
	int            currentAttackType;
	std::string    currentAttackName;
	float          currentAttackDamage;
	float          playerAttackPower;
	bool           playerHit;

	int   playerFrame, enemyFrame;
	float animTimer;

	float playerX, enemyX;
	float playerHP, playerMaxHP, enemyHP;
	float bgOffset;
	float transitionTimer, stateTimer, waveMessageTimer;
	float enemyAttackTimer, attackFeedbackTimer;
	bool  isTransitioning;
};

#endif