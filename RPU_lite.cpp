#include "Game.h"
#include "HardwareMap.h" 
#include "RPU.h"

// --- Constants & Definitions ---
#define NUM_SWITCHES 48
#define NUM_PRIORITY_SWITCHES 0
// EEPROM addresses are now centralized in Game.h

// --- RPU Switch Matrix Definition ---
PlayfieldAndCabinetSwitch gameSwitchArray[NUM_SWITCHES] = {
    // Switch#, Solenoid, Type, Flags, HoldTime, Score
    // Cabinet Switches
    {6, 0, 0, 0, 0, 0}, // Credit Button (software handled)
    {7, CONTSOL_DISABLE_FLIPPERS, 0, 0, 0, 0}, // Tilt
    {8, SOL_OUTHOLE, 0, 0, 20, 0}, // Outhole -> fires solenoid 10
    {16, CONTSOL_DISABLE_FLIPPERS, 0, 0, 0, 0}, // Slam Tilt
    // Playfield Switches
    {17, SOL_RIGHT_SLING, 0, 0, 10, 0}, // Right Slingshot
    {18, SOL_LEFT_SLING, 0, 0, 10, 0},  // Left Slingshot
    {25, SOL_THUMPER_BUMPER_1, 0, 0, 10, 0}, // Pop Bumper 1
    {26, SOL_THUMPER_BUMPER_2, 0, 0, 10, 0}, // Pop Bumper 2
    {27, SOL_THUMPER_BUMPER_3, 0, 0, 10, 0}, // Pop Bumper 3
    // ... all other switches would be mapped here ...
};

void SetupRpuSwitchMatrix() {
    RPU_SetupGameSwitches(NUM_SWITCHES, NUM_PRIORITY_SWITCHES, gameSwitchArray);
}

// --- 1. Game Setup, State & Persistence ---
unsigned long LectronamoRecharged::LoadHighScore() {
    return RPU_ReadULFromEEProm(ADDR_HIGH_SCORE);
}

void LectronamoRecharged::SaveHighScore() {
    if (currentScore > highScore) {
        highScore = currentScore;
        RPU_WriteULToEEProm(ADDR_HIGH_SCORE, highScore);
    }
}

void LectronamoRecharged::Initialize() {
    highScore = LoadHighScore();
    // ... reset all other game variables ...
    byte ballsPerGameSetting = RPU_ReadByteFromEEProm(ADDR_BALLS_PER_GAME);
    if (ballsPerGameSetting == 1) { this->ballsPerGame = 5; this->thumperScoreIs1000 = false; }
    else { this->ballsPerGame = 3; this->thumperScoreIs1000 = true; }
    
    SetupRpuSwitchMatrix(); // CRITICAL: Initialize the RPU switch matrix
}

void LectronamoRecharged::StartNewGame() {
    Initialize();
    currentScore = 0; ball = 1; gameState = GAME_START;
    PlayGameStartMusic();
    LaunchBall();
}

void LectronamoRecharged::DrainBall(bool isTilted) {
    // This function is now primarily for state change.
    // The actual ball advance/game over logic is in GameFlowUpdate's BONUS_COUNT state.
    gameState = BONUS_COUNT;
    if (isTilted) {
        currentBonus = 0; // Tilt forfeits bonus
    }
}

// --- 2. Sound Functions ---
void LectronamoRecharged::PlayGameStartMusic() { RPU_PlaySB100(0x0A); }
void LectronamoRecharged::PlayCoinInSound() { RPU_PlaySB100(0x05); }

// --- 3. Game Flow, Display & Switch Handling ---
void LectronamoRecharged::GameFlowUpdate() {
    // ... (GameFlowUpdate implementation from previous steps, with SaveHighScore call)
    switch(gameState) {
        // ...
        case GAME_OVER:
            SaveHighScore();
            RPU_SetDisplayMatch();
            break;
        // ...
    }
}

void LectronamoRecharged::HandleSwitches() {
    byte switchHit;
    while ((switchHit = RPU_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY) {
        AuditButtonCheck(switchHit);
        if (inAuditMode) { ProcessAuditModeSwitches(switchHit); return; }

        if (switchHit == 6) { // Credit button is still software-driven
            if (gameState == ATTRACT_MODE) {
                credits++; PlayCoinInSound();
            }
            if (credits > 0) {
                credits--; StartNewGame(); return;
            }
        }
        
        // The RPU matrix now handles Tilt, Outhole, Pop Bumpers, and Slingshots.
        // We only need to process switches that drive complex scoring and logic.
        if (gameState != BALL_IN_PLAY) continue;
        
        CheckSkillShot(switchHit);
        CheckArcSurgeCombo(switchHit);
        CheckDropTargets(switchHit);

        switch (switchHit) {
            // Note: SW_THUMPER_BUMPER cases are removed as RPU handles them.
            // The score is now defined in the switch matrix itself.
            // We only need cases for non-coil switches with scoring logic.
            case SW_STATIONARY_TARGET:
            case SW_SPINNER:
            case SW_TURNAROUND_ROLLOVER:
            // ... etc ...
                break;
        }
    }
}

// ... All other existing rule and helper functions (CheckDropTargets, UpdateDisplay, etc.) ...
