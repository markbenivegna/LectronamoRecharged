#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include "HardwareMap.h"
#include "Lamps.h"
#include "Solenoids.h"
#include "Switches.h"
#include "Adjustments.h"
#include "Display.h"
#include "Sound.h"

#define ADDR_BALLS_PER_GAME 0x0001 // Example EEPROM address
#define ADDR_SPECIAL_LIMIT  0x0002 // Example EEPROM address for Special award limit

// Define Game States
enum GameState {
    ATTRACT_MODE,
    GAME_START,
    BALL_IN_PLAY,
    BONUS_COUNT,
    GAME_OVER,
    AUDIT_MODE
};

class LectronamoRecharged {
public:
    void Initialize();
    void AdvanceState();
    void HandleSwitches();
    void UpdateLamps();
    void UpdateDisplay();
    void AddBonus(long value); 
    void HandleSaucer();
    void DrainBall(bool tilted = false);
    GameState GetGameState() const;

    // --- Lite POC Rules Prototypes ---
    void StartBallSave();
    void CheckSkillShot(int switchHit);
    void StartArcSurgeCombo();
    void CheckArcSurgeCombo(int switchHit);
    void AwardMystery();
    void UpdateLiteRules(); 
    void CheckDropTargets(int switchHit);
    
    // --- Core Rule Function Prototypes ---
    void AdvanceBonusStep();
    void AdvanceSpinnerBonus();
    void AdvanceBonusMultiplier();
    void CheckFiveBankCompletion();
    void AwardExtraBall();

    // --- Game Flow Functions ---
    void StartNewGame();
    void LaunchBall();
    void GameFlowUpdate();
    void UpdateDisplay();

    // --- Audit & Test Functions ---
    void AuditButtonCheck(int switchHit);
    void ProcessAuditModeSwitches(int switchHit);

private:
    GameState gameState;
    long currentScore;          
    int currentBonus;
    int bonusMultiplier;
    bool extraBallLit;
    bool saucerLocked;
    
    // --- Lite POC State Variables ---
    unsigned long ballSaveStartTime;
    bool isBallSaveActive;
    bool firstHitMade;          
    unsigned long arcSurgeTimerStart;
    bool arcSurgeActive;

    // --- Attract Mode State ---
    int attractPhase;
    int attractStep;

    // --- Audit & Test State ---
    unsigned long selfTestTimerStart;
    int selfTestPressCount;
    bool inAuditMode;
    int currentTestMode; // 0=Game, 1=Diagnostics, 2=Adjustments

    // --- Drop Target State Variables ---
    int threeBankCompleteCount; 
    bool threeTargetsDown[3]; // Tracks state of the three physical targets
    int fiveBankCompleteCount;
    bool fiveTargetsDown[5]; // Tracks state of the five physical targets

    // --- Gameplay State Variables ---
    int spinnerHitCount;
    bool holdBonus;
    int extraBalls;
    int ball;
    int player;
    int ballsPerGame;
    bool thumperScoreIs1000;
};

extern LectronamoRecharged game;

#endif // GAME_H
