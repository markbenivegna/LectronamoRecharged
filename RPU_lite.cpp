#include "Game.h"
#include "HardwareMap.h" 
#include "RPU.h"         

// --- Constants ---
const unsigned long BALL_SAVE_DURATION_MS = 20000;
const unsigned long ARC_SURGE_COMBO_TIME_MS = 8000;

// --- 1. Game Setup & State ---
void LectronamoRecharged::Initialize() {
    gameState = ATTRACT_MODE;
    currentScore = 0; bonusMultiplier = 1;
    ball = 0; player = 0;
    threeBankCompleteCount = 0; fiveBankCompleteCount = 0;
    spinnerHitCount = 0; holdBonus = false; extraBalls = 0;
    currentBonus = 0; attractPhase = 0; attractStep = 0;
    currentTestMode = 0; inAuditMode = false;
    specialAwardedThisBall = false;

    // Read settings from EEPROM
    byte ballsPerGameSetting = RPU_ReadByteFromEEProm(ADDR_BALLS_PER_GAME);
    if (ballsPerGameSetting == 1) { // ON = 5 balls
        this->ballsPerGame = 5;
        this->thumperScoreIs1000 = false; // 100 pts
    } else { // OFF = 3 balls
        this->ballsPerGame = 3;
        this->thumperScoreIs1000 = true; // 1000 pts
    }

    for (int i = 0; i < 3; i++) threeTargetsDown[i] = false;
    for (int i = 0; i < 5; i++) fiveTargetsDown[i] = false;
}

void LectronamoRecharged::StartNewGame() {
    Initialize();
    currentScore = 0; ball = 1; player = 1;
    gameState = GAME_START;
    LaunchBall();
}

void LectronamoRecharged::LaunchBall() {
    gameState = BALL_IN_PLAY;
    firstHitMade = false;
    specialAwardedThisBall = false; // Reset special limit for the new ball
    RPU_PushToSolenoidStack(SOL_3_BANK_RESET, 1);
    RPU_PushToSolenoidStack(SOL_5_BANK_RESET, 1);
    StartBallSave();
    RPU_PushToSolenoidStack(SOL_OUTHOLE, 1);
}

void LectronamoRecharged::DrainBall(bool isTilted) {
    gameState = BONUS_COUNT;
    if (isTilted) {
        RPU_SetDisableFlippers(true);
        currentBonus = 0;
    }
}

// --- 2. Core Rule Implementations ---
void LectronamoRecharged::StartBallSave() {
    ballSaveStartTime = millis(); 
    isBallSaveActive = true;
    RPU_SetLampState(LAMP_SHOOT_AGAIN, 2, 0, 0); 
}

void LectronamoRecharged::CheckDropTargets(int switchHit) {
    bool isThreeBank = (switchHit >= SW_3_BANK_1 && switchHit <= SW_3_BANK_3);
    bool isFiveBank = (switchHit >= SW_5_BANK_1 && switchHit <= SW_5_BANK_5);
    if (!isThreeBank && !isFiveBank) return;

    currentScore += 500;

    if (isThreeBank) {
        int targetIndex = switchHit - SW_3_BANK_1;
        if (!threeTargetsDown[targetIndex]) {
            threeTargetsDown[targetIndex] = true;
            bool allDown = true;
            for (int i = 0; i < 3; i++) if (!threeTargetsDown[i]) allDown = false;
            if (allDown) {
                currentScore += 6000;
                AdvanceBonusMultiplier();
                for (int i = 0; i < 3; i++) threeTargetsDown[i] = false;
                RPU_PushToSolenoidStack(SOL_3_BANK_RESET, 1);
            }
        }
    } else if (isFiveBank) {
        int targetIndex = switchHit - SW_5_BANK_1;
        if (!fiveTargetsDown[targetIndex]) {
            fiveTargetsDown[targetIndex] = true;
            bool allDown = true;
            for (int i = 0; i < 5; i++) if (!fiveTargetsDown[i]) allDown = false;
            if (allDown) {
                currentScore += 10000;
                CheckFiveBankCompletion();
                for (int i = 0; i < 5; i++) fiveTargetsDown[i] = false;
                RPU_PushToSolenoidStack(SOL_5_BANK_RESET, 1);
            }
        }
    }
}

void LectronamoRecharged::CheckFiveBankCompletion() {
    fiveBankCompleteCount++;
    if (fiveBankCompleteCount == 2) {
        extraBallLit = true;
        RPU_SetLampState(LAMP_EXTRA_BALL, 1);
    } else if (fiveBankCompleteCount >= 3) {
        byte specialLimitSetting = RPU_ReadByteFromEEProm(ADDR_SPECIAL_LIMIT);
        bool limitActive = (specialLimitSetting == 1);
        if (!limitActive || (limitActive && !specialAwardedThisBall)) {
            RPU_SetLampState(LAMP_SPECIAL, 1);
            specialAwardedThisBall = true;
        }
    }
}

// ... (Other rule functions like AdvanceBonusMultiplier, AwardMystery, etc. are assumed present and correct)

// --- 3. Audit & Test Mode ---
void LectronamoRecharged::ProcessAuditModeSwitches(int switchHit) {
    if (currentTestMode != 2) return;
    static int currentAudit = 1;
    if (switchHit == 0x7F) { currentAudit = (currentAudit % 5) + 1; }
    if (switchHit == 6) { /* Logic to adjust setting */ }
    RPU_SetDisplayMatch(currentAudit); 
}

void LectronamoRecharged::AuditButtonCheck(int switchHit) {
    if (switchHit == 0x7F && gameState != BALL_IN_PLAY) {
        currentTestMode = (currentTestMode + 1) % 3;
        if (currentTestMode == 0) {
            gameState = ATTRACT_MODE; inAuditMode = false; Initialize();
        } else {
            gameState = AUDIT_MODE; inAuditMode = true; selfTestPressCount = 0; RPU_ClearAllLamps();
        }
    }
}

// --- 4. Game Flow, Display & Switch Handling ---
void LectronamoRecharged::UpdateDisplay() {
    if (gameState == BONUS_COUNT) RPU_SetDisplay(0, currentBonus);
    else {
        RPU_SetDisplay(0, currentScore);
        RPU_SetDisplayBallInPlay(ball);
    }
}

void LectronamoRecharged::GameFlowUpdate() {
    static unsigned long phaseTimer = 0, bonusTimer = 0;
    unsigned long currentTime = millis();
    UpdateDisplay();

    switch (gameState) {
        case AUDIT_MODE:
            if (currentTestMode == 1 && currentTime - selfTestTimerStart > 2000) {
                selfTestTimerStart = currentTime;
                selfTestPressCount = (selfTestPressCount + 1) % 3;
                if (selfTestPressCount == 0) RPU_FlashAllLamps();
                if (selfTestPressCount == 1) RPU_FireContinuousSolenoid(selfTestPressCount);
                if (selfTestPressCount == 2) RPU_SetDisplay(0, 888888);
            }
            break;
        case BONUS_COUNT: {
            byte bonusCountdownSetting = RPU_ReadMPUSwitch(24);
            if (currentTime - bonusTimer > ((bonusCountdownSetting == 1) ? 100 : 50)) {
                bonusTimer = currentTime;
                if (currentBonus > 0) {
                    int step = (bonusCountdownSetting == 1) ? 1000 : 2000;
                    currentBonus -= step; currentScore += step;
                    if (currentBonus < 0) currentBonus = 0;
                } else {
                    if (extraBalls > 0) { extraBalls--; LaunchBall(); }
                    else {
                        ball++;
                        if (ball > this->ballsPerGame) gameState = GAME_OVER;
                        else LaunchBall();
                    }
                }
            }
            break;
        }
        case BALL_IN_PLAY: UpdateLiteRules(); break;
        case GAME_OVER: RPU_SetDisplayMatch(); break;
        // Attract mode logic is assumed present and correct
    }
}

void LectronamoRecharged::HandleSwitches() {
    byte switchHit;
    while ((switchHit = RPU_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY) {
        AuditButtonCheck(switchHit);
        if (gameState == AUDIT_MODE) { ProcessAuditModeSwitches(switchHit); return; }
        if (switchHit == 6 && gameState == ATTRACT_MODE) { StartNewGame(); return; }
        if (switchHit == 8 && gameState == BALL_IN_PLAY) { DrainBall(false); return; }
        if ((switchHit == 7 || switchHit == 16) && gameState == BALL_IN_PLAY) { DrainBall(true); return; }
        if (gameState != BALL_IN_PLAY) continue;
        
        CheckSkillShot(switchHit);
        CheckArcSurgeCombo(switchHit);
        CheckDropTargets(switchHit);

        switch (switchHit) {
            case SW_THUMPER_BUMPER_1: case SW_THUMPER_BUMPER_2: case SW_THUMPER_BUMPER_3:
                currentScore += (thumperScoreIs1000 ? 1000 : 100);
                break;
            // ... Other scoring cases remain here ...
        }
    }
}