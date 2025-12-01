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

#define ADDR_BALLS_PER_GAME 0x0001
#define ADDR_SPECIAL_LIMIT  0x0002
#define ADDR_HIGH_SCORE     0x0005

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
    void HandleSwitches();
    void UpdateDisplay();
    void DrainBall(bool isTilted = false);
    GameState GetGameState() const;
    void StartBallSave();
    void CheckSkillShot(int switchHit);
    void StartArcSurgeCombo();
    void CheckArcSurgeCombo(int switchHit);
    void AwardMystery();
    void UpdateLiteRules(); 
    void CheckDropTargets(int switchHit);
    void AdvanceBonusStep();
    void AdvanceSpinnerBonus();
    void AdvanceBonusMultiplier();
    void CheckFiveBankCompletion();
    void AwardExtraBall();
    void StartNewGame();
    void LaunchBall();
    void GameFlowUpdate();
    void AuditButtonCheck(int switchHit);
    void ProcessAuditModeSwitches(int switchHit);

    // --- Sound Functions ---
    void PlayGameStartMusic();
    void PlayCoinInSound();

    // --- Persistence Functions ---
    void SaveHighScore();
    unsigned long LoadHighScore();

private:
    GameState gameState;
    long currentScore;          
    int currentBonus;
    int bonusMultiplier;
    bool extraBallLit;
    unsigned long ballSaveStartTime;
    bool isBallSaveActive;
    bool firstHitMade;          
    unsigned long arcSurgeTimerStart;
    bool arcSurgeActive;
    int attractPhase;
    int attractStep;
    unsigned long selfTestTimerStart;
    int selfTestPressCount;
    bool inAuditMode;
    int currentTestMode;
    int threeBankCompleteCount; 
    bool threeTargetsDown[3];
    int fiveBankCompleteCount;
    bool fiveTargetsDown[5];
    int spinnerHitCount;
    bool holdBonus;
    int extraBalls;
    int ball;
    int player;
    int ballsPerGame;
    bool thumperScoreIs1000;
    bool specialAwardedThisBall;
    unsigned long highScore;
    int credits;
};

extern LectronamoRecharged game;

#endif // GAME_H
