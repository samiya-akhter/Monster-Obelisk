#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <string>
#include <fstream>
#include <iostream>
#include "CombatManager.h"
#include "RunnerGame.h"

// Basic trim function for spaces/newlines
static std::string trimString(const std::string& str) {
    size_t first = str.find_first_not_of(" \n\r\t");
    if (std::string::npos == first)
    {
        return str;
    }
    size_t last = str.find_last_not_of(" \n\r\t");
    return str.substr(first, (last - first + 1));
}

class SaveManager {
public:
    static std::string currentPlayerName;

    static bool HasSaveFile() {
        std::ifstream file("savegame.dat");
        return file.good();
    }

    static void LoadGame() {
        std::ifstream file("savegame.dat");
        if (file.is_open()) {
            std::string tempName;
            std::getline(file, tempName);
            currentPlayerName = trimString(tempName);

            CombatManager& cm = CombatManager::GetInstance();
            file >> cm.dawnUnlocked;
            file >> cm.drakeUnlocked;
            file >> cm.tower1Cleared;
            file >> cm.tower2Cleared;
            file >> cm.tower3Cleared;
            file >> cm.allTowersCleared;
            file >> cm.endgameRunnerDone;
            file >> cm.endgameMemoryDone;
            // The user wanted this for "return to title page" which implies reading boss defeated flag
            // However, after FB_VICTORY, the game is typically 'beaten' anyway. We load these items back up.
            
            // Read Potions
            file >> cm.healPotionCount;
            file >> cm.damagePotionCount;
            
            // Phase rewards tracking
            file >> cm.phase1RewardGiven;
            file >> cm.phase2RewardGiven;
            file >> cm.phase3RewardGiven;

            int coins;
            file >> coins;
            RunnerGame::GetInstance().SetAccumulatedCoins(coins);
            
            file.close();
            printf("Game loaded successfully for %s!\n", currentPlayerName.c_str());
        }
    }

    static void SaveGame(std::string playerName) {
        currentPlayerName = playerName;
        std::ofstream file("savegame.dat");
        if (file.is_open()) {
            file << currentPlayerName << "\n";
            
            CombatManager& cm = CombatManager::GetInstance();
            file << cm.dawnUnlocked << " ";
            file << cm.drakeUnlocked << " ";
            file << cm.tower1Cleared << " ";
            file << cm.tower2Cleared << " ";
            file << cm.tower3Cleared << " ";
            file << cm.allTowersCleared << " ";
            file << cm.endgameRunnerDone << " ";
            file << cm.endgameMemoryDone << " ";
            
            file << cm.healPotionCount << " ";
            file << cm.damagePotionCount << " ";
            
            file << cm.phase1RewardGiven << " ";
            file << cm.phase2RewardGiven << " ";
            file << cm.phase3RewardGiven << " ";
            
            file << RunnerGame::GetInstance().GetAccumulatedCoins() << "\n";

            file.close();
            printf("Game saved successfully for %s!\n", currentPlayerName.c_str());
        }
    }
    
    // Quick load just the name for displaying on the Selection screen
    static std::string GetSavedPlayerName() {
        if (!HasSaveFile()) return "";
        std::ifstream file("savegame.dat");
        std::string tempName;
        std::getline(file, tempName);
        file.close();
        return trimString(tempName);
    }
};

// Define the static member
std::string SaveManager::currentPlayerName = "";

#endif
