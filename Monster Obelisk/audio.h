#pragma once
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <string>
#include <iostream>

// BGM tracks
enum BGMTrack {
    BGM_NONE,
    BGM_MAIN,
    BGM_BATTLE,
    BGM_SHOP,
    BGM_EXTRA,
    BGM_WIN,
    BGM_LOSE
};

struct AudioSystem {
    static BGMTrack currentBGM;
    static bool isMusicEnabled;
    static bool isSfxEnabled;
    static bool tempMusicEnabled;
    static bool tempSfxEnabled;
    
    static void Init() {
        mciSendString("open \"Audios//Main Soundtrack.mp3\" alias bgmMain", NULL, 0, NULL);
        mciSendString("open \"Audios//Battle Soundtrack.mp3\" alias bgmBattle", NULL, 0, NULL);
        mciSendString("open \"Audios//Shopinterface Soundtrack.mp3\" alias bgmShop", NULL, 0, NULL);
        mciSendString("open \"Audios//Extra Soundtrack.mp3\" alias bgmExtra", NULL, 0, NULL);
        mciSendString("open \"Audios//Game Won Soundtrack.mp3\" alias bgmWin", NULL, 0, NULL);
        mciSendString("open \"Audios//Game Over Soundtrack.mp3\" alias bgmLose", NULL, 0, NULL);
    }

    static void PlayBGM(BGMTrack track) {
        if (currentBGM == track) return;
        
        // Stop current
        switch (currentBGM) {
            case BGM_MAIN: mciSendString("stop bgmMain", NULL, 0, NULL); break;
            case BGM_BATTLE: mciSendString("stop bgmBattle", NULL, 0, NULL); break;
            case BGM_SHOP: mciSendString("stop bgmShop", NULL, 0, NULL); break;
            case BGM_EXTRA: mciSendString("stop bgmExtra", NULL, 0, NULL); break;
            case BGM_WIN: mciSendString("stop bgmWin", NULL, 0, NULL); break;
            case BGM_LOSE: mciSendString("stop bgmLose", NULL, 0, NULL); break;
        }

        currentBGM = track;

        if (!isMusicEnabled) return;

        // Play new
        switch (currentBGM) {
            case BGM_MAIN: mciSendString("play bgmMain from 0 repeat", NULL, 0, NULL); break;
            case BGM_BATTLE: mciSendString("play bgmBattle from 0 repeat", NULL, 0, NULL); break;
            case BGM_SHOP: mciSendString("play bgmShop from 0 repeat", NULL, 0, NULL); break;
            case BGM_EXTRA: mciSendString("play bgmExtra from 0 repeat", NULL, 0, NULL); break;
            case BGM_WIN: mciSendString("play bgmWin from 0", NULL, 0, NULL); break;
            case BGM_LOSE: mciSendString("play bgmLose from 0", NULL, 0, NULL); break;
        }
    }

    static void PlaySFX(std::string name, int type) {
       if (!isSfxEnabled) return;

       std::string alias = "sfx_" + name + "_" + std::to_string(type);
       
       // Stop it first in case it's already playing
       std::string stopCmd = "stop " + alias;
       mciSendString(stopCmd.c_str(), NULL, 0, NULL);
       
       std::string playCmd = "play " + alias + " from 0";
       int err = mciSendString(playCmd.c_str(), NULL, 0, NULL);
       
       if (err != 0) {
           // If error, it might not be open yet. Open and play.
           std::string path = "image//Monster Images//";
           std::string filename = "";
           
           if (name == "Dawn" || name == "Drake" || name == "Vivi") {
                path += "Player Monsters//" + name + "//";
           } else {
                path += name + "//";
           }

           if (type == 1) {
                filename = name + " Attack SFX.mp3";
           } else if (type == 2) {
                if (name == "Rift") filename = "Rift Special Attack left.mp3";
                else filename = name + " Special Attack SFX.mp3";
           } else if (type == 3 && name == "Mogambo") {
                filename = "Mogambo Ground Smash Attack SFX.mp3";
           }
           
           if (filename == "") return;

           std::string fullPath = path + filename;
           std::string openCmd = "open \"" + fullPath + "\" alias " + alias;
           mciSendString(openCmd.c_str(), NULL, 0, NULL);
           mciSendString(playCmd.c_str(), NULL, 0, NULL);
       }
    }
};

// Define the static members
__declspec(selectany) BGMTrack AudioSystem::currentBGM = BGM_NONE;
__declspec(selectany) bool AudioSystem::isMusicEnabled = true;
__declspec(selectany) bool AudioSystem::isSfxEnabled = true;
__declspec(selectany) bool AudioSystem::tempMusicEnabled = true;
__declspec(selectany) bool AudioSystem::tempSfxEnabled = true;
