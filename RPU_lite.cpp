#include "Game.h"
#include "HardwareMap.h" 
#include "RPU.h"

// --- Constants & RPU Definitions ---
#define NUM_SWITCHES 9 
#define NUM_PRIORITY_SWITCHES 0

PlayfieldAndCabinetSwitch gameSwitchArray[NUM_SWITCHES] = {
    // { Switch ID, Solenoid ID, Hold Time }
    { SW_TILT, CONTSOL_DISABLE_FLIPPERS, 0 },
    { SW_SLAM_TILT, CONTSOL_DISABLE_FLIPPERS, 0 },
    { SW_RIGHT_SLINGSHOT, SOL_RIGHT_SLINGSHOT, 4 },
    { SW_LEFT_SLINGSHOT, SOL_LEFT_SLINGSHOT, 4 },
    { SW_THUMPER_CENTER, SOL_CENTER_THUMPER, 6 },
    { SW_THUMPER_RIGHT, SOL_RIGHT_THUMPER, 6 },
    { SW_THUMPER_LEFT, SOL_LEFT_THUMPER, 6 },
    { SW_OUTHOLE, SOL_OUTHOLE, 8 },
    { SW_SAUCER, SOL_SAUCER, 8 }
};

void SetupRpuSwitchMatrix() {
    RPU_SetupGameSwitches(NUM_SWITCHES, NUM_PRIORITY_SWITCHES, gameSwitchArray);
}

// --- 1. Game Setup, State & Persistence ---
unsigned long LectronamoRecharged::LoadHighScore() {
    unsigned long score = RPU_ReadULFromEEProm(ADDR_HIGH_SCORE);
    return score > 0 ? score : 100000; // Default high score
}

void LectronamoRecharged::SaveHighScore() {
    if (currentScore > highScore) {
        highScore = currentScore;
        RPU_WriteULToEEProm(ADDR_HIGH_SCORE, highScore);
    }
}

void LectronamoRecharged::Initialize() {
    highScore = LoadHighScore();
    byte ballsPerGameSetting = RPU_ReadByteFromEEProm(ADDR_BALLS_PER_GAME);
    if (ballsPerGameSetting == 1) { this->ballsPerGame = 5; } else { this->ballsPerGame = 3; }
    
    SetupRpuSwitchMatrix();
    
    // Reset all other game variables
    gameState = ATTRACT_MODE;
    // ... etc.
}

void LectronamoRecharged::DrainBall(bool isTilted) {
    if (isTilted) { currentBonus = 0; }
    SaveHighScore();
    gameState = BONUS_COUNT;
}

// All other functions (StartNewGame, LaunchBall, rules, etc.) are assumed here...

void LectronamoRecharged::HandleSwitches() {
    byte switchHit;
    while ((switchHit = RPU_PullFirstFromStack()) != NO_SWITCH_HIT) {
        // Audit mode checks would be here...

        bool isCoinSwitch = (switchHit == SW_COIN_1 || switchHit == SW_COIN_2 || switchHit == SW_COIN_3);

        if (isCoinSwitch && gameState == ATTRACT_MODE) {
            byte s17 = RPU_ReadByteFromEEProm(ADDR_MAX_CREDITS_17);
            byte s18 = RPU_ReadByteFromEEProm(ADDR_MAX_CREDITS_18);
            byte s19 = RPU_ReadByteFromEEProm(ADDR_MAX_CREDITS_19);
            
            // Per manual pg. 9, 000=8, 001=10, 010=15, 011=25
            int maxCredits = 8;
            if (s17 == 0 && s18 == 0 && s19 == 1) maxCredits = 10;
            else if (s17 == 0 && s18 == 1 && s19 == 0) maxCredits = 15;
            else if (s17 == 0 && s18 == 1 && s19 == 1) maxCredits = 25;

            if (credits < maxCredits) {
                credits++;
                // PlayCoinInSound();
            }
        }
        
        // Start button logic...
        
        // The RPU matrix handles most coils. We only process scoring switches.
        if (gameState != BALL_IN_PLAY) continue;
        
        switch (switchHit) {
            // Scoring cases...
        }
    }
}
