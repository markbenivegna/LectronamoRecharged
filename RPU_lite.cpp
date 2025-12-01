#include "Game.h"
#include "HardwareMap.h" 
#include "RPU.h"

// --- Constants & RPU Definitions ---
#define NUM_SWITCHES 9 // Number of switches in the matrix
#define NUM_PRIORITY_SWITCHES 0

// CRITICAL: This array maps physical switches to solenoid actions for low-latency RPU handling.
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

// --- 1. Game Setup, State & Persistence ---
unsigned long LectronamoRecharged::LoadHighScore() {
    unsigned long score = RPU_ReadULFromEEProm(ADDR_HIGH_SCORE);
    return score > 0 ? score : 100000; // Default high score of 100,000
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
    
    RPU_SetupGameSwitches(NUM_SWITCHES, NUM_PRIORITY_SWITCHES, gameSwitchArray);
    
    // Reset all other game variables
    gameState = ATTRACT_MODE;
    currentScore = 0;
    currentBonus = 0;
    // ... etc.
}

void LectronamoRecharged::StartNewGame() {
    Initialize(); // Re-initialize hardware and settings
    currentScore = 0;
    ball = 1;
    gameState = GAME_START;
    LaunchBall();
}

void LectronamoRecharged::DrainBall(bool isTilted) {
    if (isTilted) {
        currentBonus = 0; // Forfeit bonus on tilt
    }
    // Check for high score before bonus is added
    if (currentScore > highScore) {
        SaveHighScore();
    }
    gameState = BONUS_COUNT;
}

// ... All other rule, game flow, and helper functions (HandleSwitches, GameFlowUpdate, etc.)
// from the previous complete implementation are assumed to be here.
// The HandleSwitches function is now greatly simplified as the RPU matrix handles most coils.
// The GameFlowUpdate function's GAME_OVER case no longer needs the SaveHighScore call,
// as it's now handled in DrainBall before the final bonus count.