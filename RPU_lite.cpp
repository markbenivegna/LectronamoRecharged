// --- START RPU_lite.cpp Code Block ---
#include "Game.h"
#include "HardwareMap.h" 
#include "RPU.h"         

// --- Default Operator Settings (20s Ball Save, 8s Combo) ---
const unsigned long BALL_SAVE_DURATION_MS = 20000; 
const unsigned long ARC_SURGE_COMBO_TIME_MS = 8000; 
const int BALLS_PER_GAME = 3;

// --- 1. Game Setup at New Ball ---
void LectronamoRecharged::Initialize() {
    currentScore = 0;
    bonusMultiplier = 1;
    firstHitMade = false;
    arcSurgeActive = false;
    threeBankCompleteCount = 0;
    fiveBankCompleteCount = 0;
    spinnerHitCount = 0;
    holdBonus = false;
    extraBalls = 0;
    currentBonus = 0;
    ball = 0;
    player = 0;
    gameState = ATTRACT_MODE;
    attractPhase = 0;
    attractStep = 0;

    // Read Balls Per Game setting from EEPROM (emulating MPU Switch 7)
    byte ballsPerGameSetting = RPU_ReadByteFromEEProm(ADDR_BALLS_PER_GAME);
    if (ballsPerGameSetting == 1) { // ON = 5 balls
        this->ballsPerGame = 5;
        this->thumperScoreIs1000 = false; // 100 pts
    } else { // OFF = 3 balls
        this->ballsPerGame = 3;
        this->thumperScoreIs1000 = true; // 1000 pts
    }

    // Reset drop target states
    for (int i = 0; i < 3; i++) {
        threeTargetsDown[i] = false;
    }
    for (int i = 0; i < 5; i++) {
        fiveTargetsDown[i] = false;
    }
}

// --- 2. Ball Save Logic ---
void LectronamoRecharged::StartBallSave() {
    ballSaveStartTime = millis(); 
    isBallSaveActive = true;
    RPU_SetLampState(LAMP_SHOOT_AGAIN, 2, 0, 0); 
}

// --- 3. Skill Shot Logic ---
void LectronamoRecharged::CheckSkillShot(int switchHit) {
    if (firstHitMade == false) {
        if (switchHit == SW_SAUCER_KICKOUT) {
            firstHitMade = true;
            RPU_SetLampState(LAMP_SAUCER_LIT, 0); 
            currentScore += 5000;
            RPU_SetDisplay(0, currentScore); 
        } else {
            // A different switch was hit, skill shot is void
            firstHitMade = true;
            RPU_SetLampState(LAMP_SAUCER_LIT, 0); 
        }
    }
}

// --- 4. Arc Surge Combo Logic ---
void LectronamoRecharged::StartArcSurgeCombo() {
    arcSurgeTimerStart = millis();
    arcSurgeActive = true;
    RPU_SetLampState(LAMP_ARC_SURGE_5K, 2, 0, 0); 
}

void LectronamoRecharged::CheckArcSurgeCombo(int switchHit) {
    if (switchHit == SW_RIGHT_INLANE && arcSurgeActive == false) {
        StartArcSurgeCombo();
        return;
    }

    if (arcSurgeActive) {
        if (switchHit == SW_TARGET_ARC_SURGE) {
            currentScore += 10000;
            RPU_SetDisplay(0, currentScore); 
        }
        
        if (switchHit == SW_SAUCER_KICKOUT) {
            unsigned long superValue = 50000; 
            currentScore += superValue;
            RPU_SetDisplay(0, currentScore); 
            arcSurgeActive = false;
            RPU_SetLampState(LAMP_ARC_SURGE_5K, 0);
        }
    }
}

// --- 5. Core Rule Implementations ---
void LectronamoRecharged::AdvanceBonusStep() {
    if (currentBonus < 19000) {
        currentBonus += 1000;
    }
    RPU_SetDisplay(1, currentBonus); // Assuming display 1 is for bonus
}

void LectronamoRecharged::AdvanceSpinnerBonus() {
    static int spinnerHitCounter = 0;
    spinnerHitCounter++;
    if (spinnerHitCounter >= 4) {
        spinnerHitCounter = 0;
        AdvanceBonusStep();
    }
}

void LectronamoRecharged::AdvanceBonusMultiplier() {
    threeBankCompleteCount++;
    
    RPU_SetLampState(LAMP_2X, 0);
    RPU_SetLampState(LAMP_3X, 0);
    RPU_SetLampState(LAMP_5X, 0);

    if (threeBankCompleteCount == 1) {
        bonusMultiplier = 2;
        RPU_SetLampState(LAMP_2X, 1);
    } else if (threeBankCompleteCount == 2) {
        bonusMultiplier = 3;
        RPU_SetLampState(LAMP_3X, 1);
    } else { // 3rd and subsequent
        bonusMultiplier = 5;
        RPU_SetLampState(LAMP_5X, 1);
    }
}

void LectronamoRecharged::CheckFiveBankCompletion() {
    fiveBankCompleteCount++;
    if (fiveBankCompleteCount == 2) {
        extraBallLit = true;
        RPU_SetLampState(LAMP_EXTRA_BALL, 1);
    } else if (fiveBankCompleteCount >= 3) {
        byte specialLimitSetting = RPU_ReadByteFromEEProm(ADDR_SPECIAL_LIMIT);
        // TODO: Use specialLimitSetting to enforce award limit (e.g., 1 per game)
        RPU_SetLampState(LAMP_SPECIAL, 1);
    }
}

void LectronamoRecharged::AwardMystery() {
    int chance = rand() % 100;

    if (chance < 5) { // 1-5%
        extraBallLit = true;
        RPU_SetLampState(LAMP_EXTRA_BALL, 1);
    } else if (chance < 25) { // 6-25%
        AdvanceBonusMultiplier();
    } else if (chance < 50) { // 26-50%
        holdBonus = true;
    } else if (chance < 75) { // 51-75%
        currentScore += 100; // Puny Points
    } else if (chance < 90) { // 76-90%
        currentScore += 25000; // Big Points
    } else { // 91-100%
        DrainBall(true); // TILT
    }
}

void LectronamoRecharged::CheckDropTargets(int switchHit) {
    bool isThreeBank = (switchHit >= SW_3_BANK_1 && switchHit <= SW_3_BANK_3);
    bool isFiveBank = (switchHit >= SW_5_BANK_1 && switchHit <= SW_5_BANK_5);

    if (!isThreeBank && !isFiveBank) {
        return; // Not a drop target
    }

    currentScore += 500; // Score 500 for any drop target hit

    if (isThreeBank) {
        int targetIndex = switchHit - SW_3_BANK_1;
        if (!threeTargetsDown[targetIndex]) {
            threeTargetsDown[targetIndex] = true;
            
            bool allDown = true;
            for (int i = 0; i < 3; i++) {
                if (!threeTargetsDown[i]) allDown = false;
            }
            
            if (allDown) {
                currentScore += 6000;
                AdvanceBonusMultiplier();
                for (int i = 0; i < 3; i++) threeTargetsDown[i] = false;
                RPU_PushToSolenoidStack(SOL_3_BANK_RESET, 1);
            }
        }
    }

    if (isFiveBank) {
        int targetIndex = switchHit - SW_5_BANK_1;
        if (!fiveTargetsDown[targetIndex]) {
            fiveTargetsDown[targetIndex] = true;
            
            bool allDown = true;
            for (int i = 0; i < 5; i++) {
                if (!fiveTargetsDown[i]) allDown = false;
            }

            if (allDown) {
                currentScore += 10000;
                CheckFiveBankCompletion();
                for (int i = 0; i < 5; i++) fiveTargetsDown[i] = false;
                RPU_PushToSolenoidStack(SOL_5_BANK_RESET, 1);
            }
        }
    }
}

void LectronamoRecharged::AwardExtraBall() {
    if (extraBallLit) {
        extraBalls++;
        extraBallLit = false;
        RPU_SetLampState(LAMP_EXTRA_BALL, 0);
        RPU_SetLampState(LAMP_SHOOT_AGAIN, 2, 0, 0); // Pulsing
    }
}

void LectronamoRecharged::HandleSaucer() {
    AwardMystery();
    RPU_PushToSolenoidStack(SOL_SAUCER_KICKOUT, 1);
}

// --- 6. Audit & Test Mode Implementation ---

void LectronamoRecharged::ProcessAuditModeSwitches(int switchHit) {
    if (currentTestMode != 2) return; // Only runs in Adjustments mode

    static int currentAudit = 1;

    if (switchHit == 0x7F) { // SW_SELF_TEST_SWITCH
        currentAudit++;
        if (currentAudit > 5) currentAudit = 1; // Loop through 5 example audit items
    }

    if (switchHit == 6) { // SW_CREDIT_BUTTON
        // This is where logic to change the value of 'currentAudit' would go.
        // Example: RPU_WriteByteToEEProm(ADDR_AUDIT_X, newValue);
    }
    
    // Use the Match display to show the audit item number
    RPU_SetDisplayMatch(currentAudit); 
}

void LectronamoRecharged::AuditButtonCheck(int switchHit) {
    if (switchHit == 0x7F) { // RPU definition for the S33 self-test switch
        if (gameState != BALL_IN_PLAY) {
            currentTestMode = (currentTestMode + 1) % 3; // Cycle 0, 1, 2

            if (currentTestMode == 0) { // Exiting Audit to Game/Attract
                gameState = ATTRACT_MODE;
                inAuditMode = false;
                Initialize(); // Reset to a clean state
            } else {
                gameState = AUDIT_MODE;
                inAuditMode = true;
                selfTestPressCount = 0; // Reset sub-test counter
                RPU_ClearAllLamps();
            }
        }
    }
}


// --- 6. Game Flow & Display Implementation ---

void LectronamoRecharged::StartNewGame() {
    Initialize(); // Reset all game variables to a known clean state.
    currentScore = 0;
    ball = 1;
    player = 1;
    gameState = GAME_START;
    
    LaunchBall();
}

void LectronamoRecharged::LaunchBall() {
    gameState = BALL_IN_PLAY;
    firstHitMade = false;
    
    RPU_PushToSolenoidStack(SOL_3_BANK_RESET, 1);
    RPU_PushToSolenoidStack(SOL_5_BANK_RESET, 1);
    
    StartBallSave();
    
    RPU_PushToSolenoidStack(SOL_OUTHOLE, 1);
}

void LectronamoRecharged::DrainBall(bool isTilted) {
    gameState = BONUS_COUNT;
    if (isTilted) {
        RPU_SetDisableFlippers(true);
        currentBonus = 0; // TILT forfeits bonus
    }
}

void LectronamoRecharged::UpdateDisplay() {
    if (gameState == BONUS_COUNT) {
        RPU_SetDisplay(0, currentBonus);
    } else {
        RPU_SetDisplay(0, currentScore);
        RPU_SetDisplayBallInPlay(ball);
        // RPU_SetDisplayCredits(credits); // Placeholder for credits display
    }
}

// --- 7. Game Flow & Switch Handling ---

void LectronamoRecharged::GameFlowUpdate() {
    static unsigned long phaseTimer = 0;
    static unsigned long bonusTimer = 0;
    unsigned long currentTime = millis();

    UpdateDisplay(); // Keep displays constantly updated.

    switch (gameState) {
        case AUDIT_MODE:
            if (currentTestMode == 1) { // Diagnostics
                if (currentTime - selfTestTimerStart > 2000) { // 2s per test step
                    selfTestTimerStart = currentTime;
                    selfTestPressCount = (selfTestPressCount + 1) % 3; // Cycle 3 tests
                }
                if (selfTestPressCount == 0) RPU_FlashAllLamps();
                if (selfTestPressCount == 1) RPU_FireContinuousSolenoid(selfTestPressCount);
                if (selfTestPressCount == 2) RPU_SetDisplay(0, 888888);
            }
            // Adjustment display logic is handled in ProcessAuditModeSwitches
            break;
            
        case ATTRACT_MODE: {
            if (currentTime - phaseTimer < 75) return;
            phaseTimer = currentTime;
            // (Attract mode lamp show logic remains here)
            break;
        }

        case BONUS_COUNT: {
            byte bonusCountdownSetting = RPU_ReadMPUSwitch(24);
            unsigned int bonusRate = (bonusCountdownSetting == 1) ? 100 : 50; 
            if (currentTime - bonusTimer > bonusRate) {
                bonusTimer = currentTime;
                if (currentBonus > 0) {
                    int bonusStep = (bonusCountdownSetting == 1) ? 1000 : 2000;
                    currentBonus -= bonusStep;
                    currentScore += bonusStep;
                    if (currentBonus < 0) currentBonus = 0;
                } else {
                    if (extraBalls > 0) {
                        extraBalls--;
                        LaunchBall();
                    } else {
                        ball++;
                        if (ball > this->ballsPerGame) gameState = GAME_OVER;
                        else LaunchBall();
                    }
                }
            }
            break;
        }
        case BALL_IN_PLAY:
            UpdateLiteRules();
            break;
        case GAME_OVER:
            RPU_SetDisplayMatch();
            break;
        default:
            break;
    }
}

void LectronamoRecharged::HandleSwitches() {
    byte switchHit;
    
    while ((switchHit = RPU_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY) {
        
        AuditButtonCheck(switchHit); // Always check for test button first

        if (gameState == AUDIT_MODE) {
            ProcessAuditModeSwitches(switchHit);
            return; // In audit mode, no other switch logic runs
        }

        // --- System-level switches ---
        if (switchHit == 6 && gameState == ATTRACT_MODE) {
            StartNewGame();
            return; 
        }
        if (switchHit == 8 && gameState == BALL_IN_PLAY) {
            DrainBall(false);
            return; 
        }
        if ((switchHit == 7 || switchHit == 16) && gameState == BALL_IN_PLAY) {
            DrainBall(true);
            return;
        }

        // --- Gameplay switches ---
        if (gameState != BALL_IN_PLAY) {
            continue; 
        }

        CheckSkillShot(switchHit);
        CheckArcSurgeCombo(switchHit);
        CheckDropTargets(switchHit);

        switch (switchHit) {
            case SW_THUMPER_BUMPER_1: case SW_THUMPER_BUMPER_2: case SW_THUMPER_BUMPER_3:
                currentScore += (thumperScoreIs1000 ? 1000 : 100);
                break;
            case SW_STATIONARY_TARGET:
                currentScore += 5000;
                AdvanceBonusStep();
                RPU_SetLampState(LAMP_SAUCER_LIT, 1); 
                RPU_SetLampState(LAMP_SIDE_LANE_LIT, 1);
                break;
            case SW_SPINNER:
                currentScore += 100; 
                AdvanceSpinnerBonus(); 
                break;
            case SW_TURNAROUND_ROLLOVER:
                currentScore += 100; 
                RPU_SetLampState(LAMP_LEFT_RETURN_LANE, 1);
                break;
            case SW_EXTRA_BALL_LANE:
                AwardExtraBall();
                break;
            case SW_SAUCER_KICKOUT: 
                HandleSaucer();
                break;
        }
    }
}
// --- END RPU_lite.cpp Code Block ---
