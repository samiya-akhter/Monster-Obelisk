#ifndef RUNNERGAME_H
#define RUNNERGAME_H

#include <vector>
#include <string>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include "CombatManager.h" // Needed for the old win conditions!

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
	int type; // 0=Stone/Monster, 1=Monster, 2=Crystal (Coin)

	RunnerEntity(float _x, float _y, int _w, int _h, unsigned int _tex, int _type)
		: x(_x), y(_y), width(_w), height(_h), textureID(_tex), active(true), type(_type) {}
};

// Platform Structure
struct Platform {
	float x, y, w, h;
	Platform(float _x, float _y, float _w, float _h) : x(_x), y(_y), w(_w), h(_h) {}
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
        // Note: Maintaining original path from current game for Vivi:
		LoadAnimationFrames(viviRunTextures, "Image//Monster Images//Player Monsters//Vivi//Vivi Walk Right//vivi_walk", 25);

		// Load Environment
		bgLayers[0] = iLoadImage("Image//runner_bg.png");
		bgLayers[1] = iLoadImage("Image//runner_bg.png");

		// obstacles
		LoadZeroPaddedAnimationFrames(monsterTextures, "Image//black_monster obstacle//frame_", 36);

		// Crystals / Coins
		LoadAnimationFrames(coinTextures, "Image//coin//coin", 4);

		winImageTex = iLoadImage("Image//winUpdaterRunner.jpeg");
		loseImageTex = iLoadImage("Image//loseUpdateRunner.png");

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

	void LoadZeroPaddedAnimationFrames(std::vector<unsigned int>& frames, std::string prefix, int count) {
		frames.clear();
		char path[256];
		for (int i = 0; i < count; i++) {
			sprintf_s(path, 256, "%s%03d.png", prefix.c_str(), i);
			frames.push_back(iLoadImage(path));
		}
	}

	void UnloadResources() {
	}

	void Reset() {
		playerX = 100;
		playerY = 173;
		velocityY = 0;
		isJumping = false;
		isGameOver = false;
		unlockMessageType = 0; // FROM OLD GAME
		score = 0;
		coins = 0;
		entities.clear();

		bgX1 = 0;
		bgX2 = 1000;

		currentFrame = 0;
		animTimer = 0;

		moveSpeed = 400.0f;

		platforms.clear();
		// Start with a safe solid platform
		platforms.push_back(Platform(0, 173, 2000, 173));
		nextPlatformX = 2000;
	}

	void Update(float deltaTime) {
		if (isGameOver) return;
		if (unlockMessageType > 0) return;

		// --- Background Scrolling ---
		float bgSpeed = moveSpeed;
		bgX1 -= bgSpeed * deltaTime;
		bgX2 -= bgSpeed * deltaTime;

		if (bgX1 <= -1000) bgX1 = bgX2 + 1000 - 5;
		if (bgX2 <= -1000) bgX2 = bgX1 + 1000 - 5;

		// --- Platform Management ---
		for (int i = 0; i < (int)platforms.size(); i++) {
			platforms[i].x -= moveSpeed * deltaTime;
		}
		nextPlatformX -= moveSpeed * deltaTime;

		// Remove off-screen platforms
		if (!platforms.empty() && platforms[0].x + platforms[0].w < -500) {
			platforms.erase(platforms.begin());
		}

		// Spawn new platforms continuously
		if (nextPlatformX < 1200) {
			SpawnPlatform();
		}

		// --- Player Physics ---
		playerY += velocityY * deltaTime;

		if (isJumping || velocityY != 0) {
			velocityY -= GRAVITY * deltaTime;
		}

		bool onGround = false;
		float px = playerX + 50;

		for (int i = 0; i < (int)platforms.size(); i++) {
			Platform& p = platforms[i];
			if (px > p.x && px < p.x + p.w) {
				// Ground collision if falling
				if (velocityY <= 0) {
					float oldPy = playerY - velocityY * deltaTime;
					if (oldPy >= p.y - 15 && playerY <= p.y + 15) {
						playerY = p.y;
						velocityY = 0;
						isJumping = false;
						onGround = true;
						break;
					}
				}
				else if (!isJumping) {
					// Sticking to flat ground nicely
					if (abs(playerY - p.y) < 20) {
						playerY = p.y;
						onGround = true;
						break;
					}
				}
			}
		}

		// Apply falling if stepped off a platform
		if (!onGround) {
			isJumping = true;
			if (velocityY == 0) velocityY = -1; // Force descend
		}

		// Hole detection: Game over if falling down below screen
		if (playerY < -150) {
			isGameOver = true;
		}

		// --- Entity Movement & Collision ---
		for (int i = 0; i < (int)entities.size(); i++) {
			if (!entities[i].active) continue;

			entities[i].x -= moveSpeed * deltaTime;

			if (entities[i].x + entities[i].width < -100) {
				entities[i].active = false;
			}

			// Forgiving, core-centered player collision box
			float pBoxX = playerX + 45;
			float pBoxY = playerY + 10;
			float pBoxW = 10;
			float pBoxH = 60;

			float eBoxX = entities[i].x + 10;
			float eBoxY = entities[i].y + 10;
			float eBoxW = entities[i].width - 20;
			float eBoxH = entities[i].height - 20;

			// For obstacles, make the fatal box even more forgiving
			if (entities[i].type == 0) {
				eBoxX = entities[i].x + 35;
				eBoxW = entities[i].width - 70;
				eBoxH = entities[i].height - 40;
			}

			if (pBoxX < eBoxX + eBoxW &&
				pBoxX + pBoxW > eBoxX &&
				pBoxY < eBoxY + eBoxH &&
				pBoxY + pBoxH > eBoxY) {

				if (entities[i].type == 2) { // Coin
					coins++;
					accumulatedCoins++;
					entities[i].active = false;
                    
                    int currentTower = CombatManager::GetInstance().lastTowerPlayed;
                    bool isProgression = (CombatManager::GetInstance().lives <= 0); // Only works if lives lost
                    
                    if (isProgression) {
                        if (currentTower == 1 && coins >= 5) {
                             if (!CombatManager::GetInstance().dawnUnlocked) {
                                 CombatManager::GetInstance().dawnUnlocked = true;
                                 unlockMessageType = 1; // Trigger Dawn unlock
                             }
                        } else if (currentTower == 3 && coins >= 10) {
                             if (!CombatManager::GetInstance().drakeUnlocked) {
                                 CombatManager::GetInstance().drakeUnlocked = true;
                                 unlockMessageType = 2; // Trigger Drake unlock
                             }
                        }
                    }

                    // Endgame explicitly flags `lastTowerPlayed = 4`
                    if (CombatManager::GetInstance().allTowersCleared
                        && !CombatManager::GetInstance().endgameRunnerDone
                        && currentTower == 4
                        && coins >= 10) {
                        CombatManager::GetInstance().endgameRunnerDone = true;
                        unlockMessageType = 3; // Trigger endgame runner completion
                    }
				}
				else { // Obstacle
					isGameOver = true;
				}
			}
		}

		// --- Animation ---
		animTimer += deltaTime;
		if (animTimer >= 0.08f) {
			animTimer = 0;
			currentFrame++;
		}

		// Endless difficulty scaling
		moveSpeed += 5 * deltaTime;
	}

	void SpawnPlatform() {
		int r = rand() % 100;

		float pWidth = 300 + rand() % 500;
		float pTopY = 173;
		float pHeight = 173; // solid to floor
		float pX = nextPlatformX;

		if (r < 25) {
			// Hole (gap in path)
			pX += 150 + rand() % 200;
		}
		else if (r < 55) {
			// Elevated platform (path over actual path)
			pTopY = 173 + 100 + rand() % 60;
			pWidth = 200 + rand() % 350;
			pHeight = 35; // Hovering path
			pX += 80 + rand() % 120;

			// Random chance to also have ground directly below 
			// the elevated platform
			if (rand() % 100 < 50) {
				platforms.push_back(Platform(pX - 50, 173, pWidth + 100, 173));
			}
		}
		else {
			// Normal ground platform with occasionally small gaps
			if (rand() % 100 < 20) pX += 80 + rand() % 80;
		}

		platforms.push_back(Platform(pX, pTopY, pWidth, pHeight));

		SpawnOnPlatform(pX, pTopY, pWidth);

		float highestX = pX + pWidth;
		if (highestX > nextPlatformX) {
			nextPlatformX = highestX;
		}
	}

	void SpawnOnPlatform(float pX, float pTopY, float pWidth) {
		if (pWidth < 180) return;

		int r = rand() % 100;
		if (r < 50) {
			// Spawn Coins
			int numCoins = 1 + rand() % 3;
			for (int i = 0; i < numCoins; ++i) {
				float cX = pX + (pWidth / (numCoins + 1)) * (i + 1);
				float cY = pTopY + 20 + rand() % 40;
				entities.push_back(RunnerEntity(cX, cY, 40, 40, crystalTex, 2));
			}
		}
		else if (r < 80) {
			// Spawn Obstacle
			float oX = pX + pWidth / 2 - 50;
			entities.push_back(RunnerEntity(oX, pTopY, 100, 100, 0, 0));

			// Sometimes put a coin floating over an obstacle to encourage jumping
			if (rand() % 100 < 50) {
				entities.push_back(RunnerEntity(oX, pTopY + 140, 40, 40, crystalTex, 2));
			}
		}
	}

	void Render() {
		// Draw Background
		iShowImage((int)bgX1, 0, 1000, 600, bgLayers[0]);
		iShowImage((int)bgX2, 0, 1000, 600, bgLayers[1]);

		// Draw Black Paths (Platforms)
		iSetColor(0, 0, 0);
		for (int i = 0; i < (int)platforms.size(); i++) {
			iFilledRectangle(platforms[i].x, platforms[i].y - platforms[i].h, platforms[i].w, platforms[i].h);
		}

		// Draw Player
		unsigned int tex;
		std::vector<unsigned int>* currentAnim;

		if (currentCharacter == 0) { // Kael
			if (isJumping) currentAnim = &kaelJumpTextures;
			else currentAnim = &kaelRunTextures;
		}
		else { // Vivi
			currentAnim = &viviRunTextures;
		}

		if (!currentAnim->empty()) {
			tex = (*currentAnim)[currentFrame % currentAnim->size()];
			if (isJumping && currentCharacter == 0) {
				iShowImage((int)playerX - 7, (int)playerY, 115, 115, tex);
			}
			else {
				iShowImage((int)playerX, (int)playerY, 100, 100, tex);
			}
		}

		// Draw Entities
		for (int i = 0; i < (int)entities.size(); i++) {
			if (entities[i].active) {
				unsigned int drawTex = entities[i].textureID;
				if (entities[i].type == 2 && !coinTextures.empty()) {
					drawTex = coinTextures[currentFrame % coinTextures.size()];
				}
				else if (entities[i].type == 0 && !monsterTextures.empty()) {
					drawTex = monsterTextures[currentFrame % monsterTextures.size()];
				}

				if (drawTex != 0) {
					iShowImage((int)entities[i].x, (int)entities[i].y, entities[i].width, entities[i].height, drawTex);
				}
				else if (entities[i].type == 0) {
					// Fallback to red rectangle if texture is missing
					iSetColor(255, 50, 50);
					iFilledRectangle((int)entities[i].x, (int)entities[i].y, entities[i].width, entities[i].height);
				}
			}
		}

		// UI FROM OLD GAME
		char msg[128];
		sprintf_s(msg, 128, "Run Coins: %d | Total: %d", coins, accumulatedCoins);
		iSetColor(255, 215, 0); // Gold
		iText(740, 550, msg, (void*)0x0008);

        if (unlockMessageType == 1) { // Dawn
            iSetColor(0, 0, 0);
            iFilledRectangle(200, 200, 600, 200);
            iSetColor(0, 255, 0);
            iText(320, 320, "New Monster Dawn Unlocked!", (void*)0x0006); 
            iSetColor(255, 255, 255);
            iText(300, 280, "Press SPACE to equip and return to Tower 1", (void*)0x0008);
        }
        else if (unlockMessageType == 2) { // Drake
            iSetColor(0, 0, 0);
            iFilledRectangle(200, 200, 600, 200);
            iSetColor(0, 255, 0);
            iText(320, 320, "New Monster Drake Unlocked!", (void*)0x0006); 
            iSetColor(255, 255, 255);
            iText(300, 280, "Press SPACE to equip and return to Tower 3", (void*)0x0008);
        }
        else if (unlockMessageType == 3) { // Endgame runner done
            iSetColor(0, 0, 0);
            iFilledRectangle(150, 200, 700, 200);
            iSetColor(255, 215, 0);
            iText(280, 340, "Runner Quest Complete! (10 Crystals)", (void*)0x0006);
            iSetColor(255, 255, 255);
            iText(250, 290, "Now complete the Memory Game to finish your journey!", (void*)0x0008);
            iText(320, 265, "Press SPACE to go to the Memory Game", (void*)0x0008);
        }

		if (isGameOver) {
			iShowImage(0, 0, 1000, 600, loseImageTex);
		}
	}

	void HandleInput(unsigned char key) {
        // OLD GAME HANDLE INPUT FOR WIN STATES
        if (unlockMessageType > 0) {
            if (key == ' ') {
                extern int gameState; 
                if (unlockMessageType == 1) {
                    gameState = 6; // Return to Tower 1 
                    CombatManager::GetInstance().InitCombatWithDawn();
                } else if (unlockMessageType == 2) {
                    gameState = 9; // Return to Tower 3 
                    CombatManager::GetInstance().InitTower3WithDrake();
                } else if (unlockMessageType == 3) {
                    // Endgame: runner done
                    extern int wildAreaMode;
                    if (CombatManager::GetInstance().endgameMemoryDone) {
                        gameState = 1; // Both done, back to outer map new image!
                    } else {
                        wildAreaMode = 0; // Return to selection screen
                        gameState = 5;
                    }
                }
                unlockMessageType = 0;
                Reset(); // Reset runner game for next time
            }
            return;
        }

		if (isGameOver) {
			if (key == ' ') {
				Reset();
			}
			return;
		}

		if (key == ' ' && !isJumping) {
			isJumping = true;
			velocityY = JUMP_FORCE;
		}
	}

	int GetAccumulatedCoins() const { return accumulatedCoins; }
	void SetAccumulatedCoins(int amount) { accumulatedCoins = amount; }
	void AddAccumulatedCoins(int amount) { accumulatedCoins += amount; }
	void SetPhase2CoinTarget(int target) { accumulatedCoins = target; } // Used for phase2CoinTarget reference

private:
	RunnerGame() :
		initialized(false),
		currentCharacter(0),
		viviUnlocked(false),
		unlockMessageType(0),
		GRAVITY(2000.0f),
		JUMP_FORCE(900.0f),
		playerX(100.0f),
		playerY(175.0f),
		velocityY(0.0f),
		isJumping(false),
		moveSpeed(400.0f),
		bgX1(0.0f),
		bgX2(1000.0f),
		score(0),
		coins(0),
		accumulatedCoins(0),
		isGameOver(false),
		currentFrame(0),
		animTimer(0.0f),
		nextPlatformX(2000.0f),
		lavaStoneTex(0),
		crystalTex(0),
		winImageTex(0),
		loseImageTex(0)
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
	std::vector<unsigned int> coinTextures;
	unsigned int bgLayers[2];
	unsigned int lavaStoneTex;
	std::vector<unsigned int> monsterTextures;
	unsigned int crystalTex; // Included for old references
	unsigned int winImageTex;
	unsigned int loseImageTex;

	// Game Variables
	const float GRAVITY;
	const float JUMP_FORCE;

	float playerX, playerY;
	float velocityY;
	bool isJumping;

	int currentCharacter; // 0=Kael, 1=Vivi
	bool viviUnlocked;
	int unlockMessageType; // OLD GAME FLAG

	float moveSpeed;
	float bgX1, bgX2;

	int score;
	int coins;
	int accumulatedCoins;
	bool isGameOver;

	// Animation
	int currentFrame;
	float animTimer;

	// World/Platforms
	std::vector<Platform> platforms;
	float nextPlatformX;

	std::vector<RunnerEntity> entities;
};

#endif
