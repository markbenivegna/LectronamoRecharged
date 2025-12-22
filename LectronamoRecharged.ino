// LectronamoRecharged.ino

#include "RPU.h"
#include "RPU_Config.h"
#include <Arduino.h>
#include "Lectronamo.h"

GameState gameState;
long playerScores[4];
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
int gNumPlayers = 1;
bool specialAwardedThisBall;
unsigned long highScore;
int credits;
uint8_t gGameFlags = 0;
byte MaxTiltWarnings;
byte NumTiltWarnings = 0;
long ExtraBallScoreValue;
long SpecialScoreValue;
byte AwardHighscoreNumReplays;
unsigned long lastSwitchHitTime = 0;
unsigned long gAttractModeTimer = 0;
int gSpinnerAdvancerCount = 0;
int gChaserIndex = 0;

// Attract Mode Lamp Arrays
const byte SpinnerAdvanceLamps[] = {
    LAMP_ADVANCE_BONUS_1, LAMP_ADVANCE_BONUS_2, LAMP_ADVANCE_BONUS_3, LAMP_ADVANCE_BONUS_4
};
const int NUM_SPINNER_ADVANCE_LAMPS = 4;

const byte BonusLadderLamps[] = {
    LAMP_BONUS_1K, LAMP_BONUS_2K, LAMP_BONUS_3K, LAMP_BONUS_4K, 
    LAMP_BONUS_5K, LAMP_BONUS_6K, LAMP_BONUS_7K, LAMP_BONUS_8K, 
    LAMP_BONUS_9K, LAMP_BONUS_10K
};
const int NUM_BONUS_LADDER_LAMPS = 10;

const byte AllFeatureLamps[] = {
    LAMP_BONUS_1K, LAMP_BONUS_2K, LAMP_BONUS_3K, LAMP_BONUS_4K, 
    LAMP_BONUS_5K, LAMP_BONUS_6K, LAMP_BONUS_7K, LAMP_BONUS_8K, 
    LAMP_BONUS_9K, LAMP_BONUS_10K,
    LAMP_BONUS_MULTIPLIER_2X, LAMP_BONUS_MULTIPLIER_3X, LAMP_BONUS_MULTIPLIER_5X,
    LAMP_SAUCER, LAMP_ADVANCE_BONUS_3, LAMP_EXTRA_BALL_LANE, 
    LAMP_SPINNER, LAMP_SHOOT_AGAIN
};
const int NUM_ALL_FEATURE_LAMPS = sizeof(AllFeatureLamps) / sizeof(AllFeatureLamps[0]);

// Helper Functions
void StopAttractModeLights();
void RunAttractModeLights(unsigned long CurrentTime);

// RPU Hardware Mapping
#define NUM_SWITCHES 40 // Total number of switches for the RPU to scan
#define NUM_PRIORITY_SWITCHES 5 // Number of switches with immediate solenoid responses

PlayfieldAndCabinetSwitch gameSwitchArray[NUM_PRIORITY_SWITCHES] = {
    { SW_RIGHT_SLINGSHOT, SOL_RIGHT_SLINGSHOT, 4 },
    { SW_LEFT_SLINGSHOT, SOL_LEFT_SLINGSHOT, 4 },
    { SW_THUMPER_CENTER, SOL_CENTER_THUMPER, 6 },
    { SW_THUMPER_RIGHT, SOL_RIGHT_THUMPER, 6 },
    { SW_THUMPER_LEFT, SOL_LEFT_THUMPER, 6 }
};

// Core RPU Callbacks & Setup
void SaveHighScore() {
    if (playerScores[player-1] > highScore) {
        highScore = playerScores[player-1];
        RPU_WriteULToEEProm(ADDR_HIGH_SCORE, highScore);
    }
}

void DrainBall(bool isTilted = false) {
    if (isTilted) {
        currentBonus = 0;
    }
    gGameFlags &= ~FLAG_SIDE_LANE_LIT;
    RPU_SetLampState(LAMP_SAUCER, 0);
    RPU_SetDisableFlippers(true);
    SaveHighScore();
    PlayBonusCollectSound();
    gameState = BONUS_COUNT;
}

void CheckAndResetSolenoids() {
    if (RPU_ReadSingleSwitchState(SW_SAUCER)) {
        FireSolenoid(SOL_SAUCER, 50); // Eject Saucer (Sol 14)
    }
    if (RPU_ReadSingleSwitchState(SW_KICKER)) {
        FireSolenoid(SOL_KICKER, 50);  // Eject Side Lane (Sol 12)
    }
    if (threeTargetsDown[0] || threeTargetsDown[1] || threeTargetsDown[2]) {
        FireSolenoid(SOL_DROP_TARGET_3BANK_RESET, 100); 
    }
    if (fiveTargetsDown[0] || fiveTargetsDown[1] || fiveTargetsDown[2] || fiveTargetsDown[3] || fiveTargetsDown[4]) {
        FireSolenoid(SOL_DROP_TARGET_5BANK_RESET, 100); 
    }
}

// Core Game Flow & Switch Dispatcher
void AddToPlayerScore(long score) {
    if (player > 0) playerScores[player-1] += score;
}

void PlayStockSound(byte soundID) {
    RPU_PlaySB100(soundID);
}

void AwardReplay() {
    credits++; // Increment credit
    RPU_SetCoinLockout(credits >= 8);
    PlayStockSound(SND_10000_POINTS); 
    FireSolenoid(SOL_KNOCKER, 75); 
}

void PlayCreditAddMelody() {
    PlayStockSound(SND_10000_POINTS); // Sol 4 (Lowest Pitch)
    delay(100); 
    PlayStockSound(SND_100_POINTS); // Sol 2 
    delay(100); 
    PlayStockSound(SND_1000_POINTS); // Sol 3
    delay(100); 
    PlayStockSound(SND_10_POINTS); // Sol 1 (Highest Pitch)
}

void PlayGameStartMelody() {
    for (int i = 0; i < 2; i++) {
        PlayStockSound(SND_10_POINTS); // Sol 1 (Highest Pitch)
        delay(100); 
        PlayStockSound(SND_1000_POINTS); // Sol 3 
        delay(100); 
        PlayStockSound(SND_100_POINTS); // Sol 2 
        delay(100); 
        PlayStockSound(SND_10000_POINTS); // Sol 4 (Lowest Pitch)
        delay(300); // Longer pause before repeat
    }
}

void PlayBonusCollectSound() {
    PlayStockSound(SND_ADD_BONUS);
    delay(500);
    PlayStockSound(SND_10000_POINTS);
}

void PlayExtraBallAward() {
    PlayStockSound(SND_10000_POINTS);
    delay(100);
    PlayStockSound(SND_POP_BUMPER);
    delay(100);
    PlayStockSound(SND_10000_POINTS);
}

void UpdatePlayerDisplay() {
    if (gameState == BALL_IN_PLAY) {
        byte displayValue = (player * 10) + ball;
        RPU_SetDisplay(3, displayValue, true); 
    } else if (gameState == GAME_OVER || gameState == ATTRACT_MODE) {
        RPU_SetDisplay(3, 0, true);
    }
}

void CheckHighScores() {
    byte AwardHighscoreNumReplays = RPU_ReadByteFromEEProm(ADDR_HIGHSCORE_REPLAY_AWARD);
    if (AwardHighscoreNumReplays > 3) AwardHighscoreNumReplays = 0; 
    
    long gameHighScore = 0;
    for (int i = 0; i < gNumPlayers; i++) {
        if (playerScores[i] > gameHighScore) {
            gameHighScore = playerScores[i];
        }
    }

    if (gameHighScore > highScore) {
        highScore = gameHighScore;
        RPU_WriteULToEEProm(ADDR_HIGH_SCORE, highScore);
        
        if (AwardHighscoreNumReplays > 0) {
            for (byte i = 0; i < AwardHighscoreNumReplays; i++) {
                AwardReplay();
                delay(300); 
            }
        }
    }
}

void RunMatchMode(unsigned long CurrentTime) {
    static unsigned long matchStartTime = 0;
    static byte matchDigit = 0xFF;

    if (gameState != MATCH_MODE) {
        matchStartTime = 0;
        return;
    }

    if (matchStartTime == 0) {
        matchStartTime = CurrentTime;
        matchDigit = random(0, 10);
        RPU_SetDisplayMatch(matchDigit, true);
    }

    if (CurrentTime < matchStartTime + TIME_MATCH_SEQUENCE_MS) { 
        return;
    }

    bool matchFound = false;
    for (int p = 1; p <= gNumPlayers; p++) {
        long lastTwoDigits = playerScores[p - 1] % 100;
        byte playerMatchDigit = (byte)(lastTwoDigits / 10);
        
        if (playerMatchDigit == matchDigit) {
            matchFound = true;
            AwardReplay();
        }
    }
    
    gameState = HIGH_SCORE_CHECK;
}

void RunBonusCountdown(unsigned long CurrentTime) {
    static unsigned long lastBonusStepTime = 0;
    static int bonusStepsRemaining = 0;
    static long bonusToAward = 0;
    static long bonusAwarded = 0;
    static int currentBonusLampIndex = NUM_BONUS_LADDER_LAMPS - 1;
    const unsigned long STEP_INTERVAL = 150; // 150ms per bonus step

    if (gameState != BONUS_COUNT) {
        // Reset state when leaving BONUS_COUNT
        bonusStepsRemaining = 0;
        return;
    }

    if (bonusStepsRemaining == 0 && lastBonusStepTime == 0) {
        bonusToAward = (long)currentBonus * bonusMultiplier;
        bonusStepsRemaining = currentBonus / 1000; 
        bonusAwarded = 0;
        currentBonusLampIndex = (currentBonus / 1000) - 1;
        
        for (int i = 0; i <= currentBonusLampIndex && i < NUM_BONUS_LADDER_LAMPS; i++) {
             RPU_SetLampState(BonusLadderLamps[i], 1);
        }
        
        lastBonusStepTime = CurrentTime; 
    }

    if (CurrentTime < lastBonusStepTime + STEP_INTERVAL) {
        return;
    }
    
    if (bonusStepsRemaining <= 0) {
        if (bonusAwarded < bonusToAward) {
            AddToPlayerScore(bonusToAward - bonusAwarded);
        }
        gameState = BALL_IN_PLAY; 
        FireSolenoid(SOL_OUTHOLE, 150);
        currentBonus = 1000;
        lastBonusStepTime = 0;
        return;
    }

    if (currentBonusLampIndex >= 0 && currentBonusLampIndex < NUM_BONUS_LADDER_LAMPS) {
         RPU_SetLampState(BonusLadderLamps[currentBonusLampIndex], 0);
    }
    long scoreStep = 1000L * bonusMultiplier;
    
    if (bonusToAward - bonusAwarded < scoreStep) {
        scoreStep = bonusToAward - bonusAwarded; 
    }
    
    AddToPlayerScore(scoreStep); 
    bonusAwarded += scoreStep;
    PlayStockSound(SND_100_POINTS);
    
    bonusStepsRemaining--;
    currentBonusLampIndex--;
    
    lastBonusStepTime = CurrentTime;
}

void RunBonusLadderChase(unsigned long CurrentTime) {
    static unsigned long lastChaseTime = 0;
    const unsigned long CHASE_INTERVAL = 50;

    if ((gGameFlags & FLAG_ARC_SURGE_ACTIVE) && CurrentTime > lastChaseTime + CHASE_INTERVAL) {
        RPU_SetLampState(BonusLadderLamps[gChaserIndex], 0);
        gChaserIndex = (gChaserIndex + 1) % NUM_BONUS_LADDER_LAMPS;
        RPU_SetLampState(BonusLadderLamps[gChaserIndex], 1);
        lastChaseTime = CurrentTime;
    } else if (!(gGameFlags & FLAG_ARC_SURGE_ACTIVE)) {
        RPU_SetLampState(BonusLadderLamps[gChaserIndex], 0);
    }
}

void RunBallSearch(unsigned long CurrentTime) {
    const unsigned long BALL_SEARCH_TIMEOUT_MS = 15000; 

    if (gameState == BALL_IN_PLAY && firstHitMade) {
        if (CurrentTime > lastSwitchHitTime + BALL_SEARCH_TIMEOUT_MS) {
            FireSolenoid(SOL_SAUCER, 50); 
            FireSolenoid(SOL_KICKER, 50);  
            FireSolenoid(SOL_CENTER_THUMPER, 20); 
            FireSolenoid(SOL_RIGHT_THUMPER, 20);
            FireSolenoid(SOL_LEFT_THUMPER, 20); 
            lastSwitchHitTime = CurrentTime; 
        }
    }
}


void FireSolenoid(byte sol, int holdTime) {
    RPU_PushToSolenoidStack(sol, holdTime);
}

void HandleSpinnerHit() {
    spinnerHitCount++;
    PlayStockSound(SND_100_POINTS);
    
    long score = SCORE_SPINNER_BASE; 

    if (currentBonus >= 10000) {
        score = SCORE_SPINNER_LIT; // 1,000 points
        RPU_SetLampState(LAMP_SPINNER, 1); // Turn ON Spinner Lit Lamp
    } else {
        RPU_SetLampState(LAMP_SPINNER, 0); // Turn OFF Spinner Lit Lamp
    }
    AddToPlayerScore(score); 

    if (spinnerHitCount % 4 == 0) {
        currentBonus += 1000; 
        if (currentBonus > 19000) {
            currentBonus = 19000;
        }
    }

    RPU_SetDisplay(0, spinnerHitCount, true);
}

// Core Ruleset Handlers
void ClearAllMultiplierLamps() {
    RPU_SetLampState(LAMP_BONUS_MULTIPLIER_2X, 0, 0, 0);
    RPU_SetLampState(LAMP_BONUS_MULTIPLIER_3X, 0, 0, 0);
    RPU_SetLampState(LAMP_BONUS_MULTIPLIER_5X, 0, 0, 0);
}

// Handles 3-Bank completion and bonus multiplier logic.
void Handle3BankCompletion() {
    AddToPlayerScore(SCORE_3BANK_COMPLETION); 
    FireSolenoid(SOL_DROP_TARGET_3BANK_RESET, 100); 
    
    threeBankCompleteCount++;
    for (int i = 0; i < 3; i++) { threeTargetsDown[i] = false; }

    ClearAllMultiplierLamps();
    
    if (threeBankCompleteCount == 1) {
        bonusMultiplier = 2;
        RPU_SetLampState(LAMP_BONUS_MULTIPLIER_2X, 1, 0, 0); 
    } else if (threeBankCompleteCount == 2) {
        bonusMultiplier = 3;
        RPU_SetLampState(LAMP_BONUS_MULTIPLIER_3X, 1, 0, 0); 
    } else if (threeBankCompleteCount >= 3) {
        bonusMultiplier = 5;
        RPU_SetLampState(LAMP_BONUS_MULTIPLIER_5X, 1, 0, 0); 
    }
}

void Handle5BankCompletion() {
    AddToPlayerScore(SCORE_5BANK_COMPLETION);
    FireSolenoid(SOL_DROP_TARGET_5BANK_RESET, 100); 
    
    fiveBankCompleteCount++;
    for (int i = 0; i < 5; i++) {
        fiveTargetsDown[i] = false;
    }

    byte extraBallSetting = RPU_ReadByteFromEEProm(ADDR_EXTRA_BALL_BYPASS);
    bool awardExtraBall = (extraBallSetting != 0); 

    if (fiveBankCompleteCount == 2) {
        if (awardExtraBall) { 
            RPU_SetLampState(LAMP_EXTRA_BALL_LANE, 1, 0, 0); 
            extraBallLit = true;
        } else {
            AddToPlayerScore(ExtraBallScoreValue);
        }
    } else if (fiveBankCompleteCount >= 3) {
        AddToPlayerScore(SpecialScoreValue);
        RPU_SetLampState(LAMP_BONUS_MULTIPLIER_5X, 1, 0, 500); 
        PlayExtraBallAward();
    }
}

void CollectBonus() {
    PlayBonusCollectSound();
    ClearAllMultiplierLamps();
    gameState = BONUS_COUNT;
}

// Handles all switch closures and dispatches to specific handlers.
void ProcessSwitches() {
    byte switchHit;
    while ((switchHit = RPU_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY) {
        // Update the Ball Search Timer on any switch activity
        if (!firstHitMade) {
            // Any switch hit cancels the skill shot.
            // Turn off the lamp and disable the skill shot immediately.
            RPU_SetLampState(LAMP_SAUCER, 0);
        }

        lastSwitchHitTime = millis();

        switch (switchHit) {
            case SW_OUTHOLE:
                DrainBall();
                break;

            case SW_KICKER: // Side Lane Kicker
                // Kicker 'Short Circuit' Rule: If the Arc Surge combo is active,
                // hitting the kicker cancels it before collecting bonus.
                if (gGameFlags & FLAG_ARC_SURGE_ACTIVE) {
                    gGameFlags &= ~FLAG_ARC_SURGE_ACTIVE;
                    RPU_SetLampState(LAMP_SAUCER, (gGameFlags & FLAG_SIDE_LANE_LIT) ? 1 : 0);
                }
                CollectBonus();
                FireSolenoid(SOL_KICKER, 50); // Fire Solenoid 12
                break;
            
            case SW_TARGET_1_3BANK: threeTargetsDown[0] = true; AddToPlayerScore(SCORE_DROP_TARGET_BASE); PlayStockSound(SND_100_POINTS); break;
            case SW_TARGET_2_3BANK: threeTargetsDown[1] = true; AddToPlayerScore(SCORE_DROP_TARGET_BASE); PlayStockSound(SND_100_POINTS); break;
            case SW_TARGET_3_3BANK: threeTargetsDown[2] = true; AddToPlayerScore(SCORE_DROP_TARGET_BASE); PlayStockSound(SND_100_POINTS); break;

            case SW_TARGET_1_5BANK: fiveTargetsDown[0] = true; AddToPlayerScore(SCORE_DROP_TARGET_BASE); PlayStockSound(SND_100_POINTS); break;
            case SW_TARGET_2_5BANK: fiveTargetsDown[1] = true; AddToPlayerScore(SCORE_DROP_TARGET_BASE); PlayStockSound(SND_100_POINTS); break;
            case SW_TARGET_3_5BANK: fiveTargetsDown[2] = true; AddToPlayerScore(SCORE_DROP_TARGET_BASE); PlayStockSound(SND_100_POINTS); break;
            case SW_TARGET_4_5BANK: fiveTargetsDown[3] = true; AddToPlayerScore(SCORE_DROP_TARGET_BASE); PlayStockSound(SND_100_POINTS); break;
            case SW_TARGET_5_5BANK: fiveTargetsDown[4] = true; AddToPlayerScore(SCORE_DROP_TARGET_BASE); PlayStockSound(SND_100_POINTS); break;

            case SW_SPINNER:
                HandleSpinnerHit();
                break;

            case SW_BOTTOM_POP:
            case SW_RIGHT_POP:
            case SW_LEFT_POP:
                AddToPlayerScore(SCORE_POP_BUMPER);
                PlayStockSound(SND_POP_BUMPER);
                break;

            case SW_RIGHT_INLANE:
                AddToPlayerScore(100L);
                break;
            
            case SW_ADV_BONUS_1000: // Arc Surge Target 1
                if (gGameFlags & FLAG_ARC_SURGE_ACTIVE) {
                    AddToPlayerScore(SCORE_ARC_SURGE_T1);
                    PlayStockSound(SND_1000_POINTS);
                } else if (gGameFlags & FLAG_SIDE_LANE_LIT) {
                    AddToPlayerScore(5000L);
                    currentBonus += 3000;
                    PlayStockSound(SND_10000_POINTS);
                } else {
                    AddToPlayerScore(1000L);
                    currentBonus += 1000;
                    PlayStockSound(SND_1000_POINTS);
                }
                break;

            case SW_SAUCER: // Saucer / Eject Pocket
                HandleSkillShot(switchHit); // Handles both Skill Shot and normal saucer logic
                break;

            case SW_STANDUP_TARGET:
                gGameFlags |= FLAG_SIDE_LANE_LIT;
                RPU_SetLampState(LAMP_SAUCER, 1);
                AddToPlayerScore(500);
                break;

            case SW_RIGHT_SLING: // SW 38
            case SW_LEFT_SLING:  // SW 39
                AddToPlayerScore(100L); // Corrected to 100 points
                PlayStockSound(SND_100_POINTS); // Use the 100 pt sound
                break;
            
            case SW_ADV_BONUS_300: // SW 18 is the 300 Advance Bonus switch (Rebound Rubber)
                AddToPlayerScore(300L); // 300 points
                currentBonus += 1000;    // 1 bonus step advance
                PlayStockSound(SND_100_POINTS); // Use the 100 pt sound (chirp)
                break;

            case SW_SCORE_10: // SW 19
                AddToPlayerScore(10L);
                PlayStockSound(SND_10_POINTS); // Use the 10 pt sound (Solenoid 1)
                break;

            case SW_RIGHT_OUTLANE:
            case SW_LEFT_OUTLANE:
                AddToPlayerScore(SCORE_OUTLANE); // 3,000 points
                currentBonus += 3000;            // 3 bonus advances
                PlayStockSound(SND_1000_POINTS); // ADDED: High-value sound for outlane
                break;

            case SW_ROLLOVER_BUTTON:
                AddToPlayerScore(SCORE_SPINNER_BASE); // 100 points
                gGameFlags |= FLAG_LEFT_RETURN_LANE_LIT; // Lite the Left Return Lane
                PlayStockSound(SND_100_POINTS); // ADDED: Standard 100 pt sound
                break;

            case SW_LEFT_INLANE:
                if (gGameFlags & FLAG_LEFT_RETURN_LANE_LIT) { // This flag name seems ok to keep
                    AddToPlayerScore(9000L);
                    gGameFlags &= ~FLAG_LEFT_RETURN_LANE_LIT; // Turn off the light
                    PlayStockSound(SND_10000_POINTS); // ADDED: Very high-value sound
                } else {
                    AddToPlayerScore(SCORE_SPINNER_BASE); 
                    PlayStockSound(SND_100_POINTS); // ADDED: Standard 100 pt sound
                }
                break;
        }

        if (!firstHitMade) firstHitMade = true;
    }

    if (threeTargetsDown[0] && threeTargetsDown[1] && threeTargetsDown[2]) {
        Handle3BankCompletion();
    }
    if (fiveTargetsDown[0] && fiveTargetsDown[1] && fiveTargetsDown[2] && fiveTargetsDown[3] && fiveTargetsDown[4]) {
        Handle5BankCompletion();
    }
}

// Custom Ruleset Implementation
void HandleSkillShot(byte switchHit) {
    if (gameState == BALL_IN_PLAY && !firstHitMade) {
        if (switchHit == SW_SAUCER) {
            AddToPlayerScore(5000L);
            currentBonus += 3000;
            PlayStockSound(SND_10000_POINTS);
            FireSolenoid(SOL_SAUCER, 50);
        }
        firstHitMade = true; // Prevents subsequent Skill Shots
        return;
    }
    
    if (switchHit == SW_SAUCER) {
        if (gGameFlags & FLAG_SIDE_LANE_LIT) { // Check if lit by stationary target
            AddToPlayerScore(SCORE_SKILL_SHOT); // 5,000 points
            PlayStockSound(SND_10000_POINTS); // Use high-value sound
            // Per rules, do NOT clear the flag or turn off the lamp. It stays lit until drain.
        } else {
            AddToPlayerScore(500L);  // 500 points
            currentBonus += 1000;    // 1 bonus advance
            PlayStockSound(SND_1000_POINTS); // Base sound
        }
        FireSolenoid(SOL_SAUCER, 50);
    }
}

void RunBallSaveLogic(unsigned long CurrentTime) {
    if (isBallSaveActive) {
        RPU_SetLampState(LAMP_SHOOT_AGAIN, 1, 0, 500); // Visual cue (Pulse)
        
        if (CurrentTime > ballSaveStartTime + TIME_BALL_SAVE_DURATION_MS) {
            isBallSaveActive = false;
            RPU_SetLampState(LAMP_SHOOT_AGAIN, 0);
        }
    }
}

void HandleArcSurgeCombo(unsigned long CurrentTime) {
    // 1. COMBO START (Right Inlane Hit)
    if (RPU_ReadSingleSwitchState(SW_RIGHT_INLANE) && !(gGameFlags & FLAG_ARC_SURGE_ACTIVE)) {
        gGameFlags |= FLAG_ARC_SURGE_ACTIVE;
        arcSurgeTimerStart = CurrentTime;

        // Start pulsing LAMP_SAUCER immediately
        RPU_SetLampState(LAMP_SAUCER, 1, 0, 500); 
    }

    if (gGameFlags & FLAG_ARC_SURGE_ACTIVE) {
        // 3. COMBO SUCCESS (Saucer Hit) - End Combo
        if (RPU_ReadSingleSwitchState(SW_SAUCER)) {
            // Score Super Value and Add 3 bonus advances
            AddToPlayerScore(SCORE_ARC_SURGE_SUPER);
            currentBonus += 3000; 
            PlayStockSound(SND_10000_POINTS); 

            // Cleanup flags and lamps immediately
            gGameFlags &= ~FLAG_ARC_SURGE_ACTIVE;
            RPU_SetLampState(LAMP_SAUCER, 0);
            
            // Fire Saucer Solenoid on successful completion
            FireSolenoid(SOL_SAUCER, 50); 
            arcSurgeTimerStart = 0; // Reset timer to prevent timeout logic from running
        }

        // 4. COMBO END/TIMEOUT (Cleanup Check)
        if (arcSurgeTimerStart != 0 && (CurrentTime > arcSurgeTimerStart + TIME_ARC_SURGE_COMBO_MS)) {
            // Cleanup flags: This will also stop the chase in RunBonusLadderChase()
            gGameFlags &= ~FLAG_ARC_SURGE_ACTIVE;
            
            // Ensure LAMP_SAUCER is OFF
            if (gGameFlags & FLAG_SIDE_LANE_LIT) {
                RPU_SetLampState(LAMP_SAUCER, 1);
            } else {
                RPU_SetLampState(LAMP_SAUCER, 0);
            }
            arcSurgeTimerStart = 0;
        }
    }
}

void StartGame(byte numPlayers) {
    for(int i=0; i<4; i++) { playerScores[i] = 0; }
    currentBonus = 1000; // Start with 1000 bonus
    bonusMultiplier = 1; // Reset Multiplier
    extraBallLit = false;
    isBallSaveActive = true; 
    ballSaveStartTime = millis(); 
    firstHitMade = false;
    arcSurgeActive = false;
    threeBankCompleteCount = 0;
    fiveBankCompleteCount = 0;
    spinnerHitCount = 0;
    holdBonus = false;
    extraBalls = 0;
    ball = 1;
    player = 1;
    gNumPlayers = numPlayers; 
    StopAttractModeLights();
    
    RPU_SetDisableFlippers(false);
    gameState = BALL_IN_PLAY; 
    PlayGameStartMelody();
    
}

void RPU_Callback_GameLogic(){
    unsigned long CurrentTime = millis(); 
    
    // --- 1. Hard Reset Timer Logic ---
    static unsigned long startButtonHoldTime = 0;
    const unsigned long HOLD_TIME_TO_RESET_MS = 2000; 

    if (gameState == BALL_IN_PLAY && ball > 1) { 
        if (RPU_ReadSingleSwitchState(SW_CREDIT_BUTTON)) { 
            if (startButtonHoldTime == 0) startButtonHoldTime = CurrentTime; 
            else if (CurrentTime > startButtonHoldTime + HOLD_TIME_TO_RESET_MS) {
                StartGame(1); 
                startButtonHoldTime = 0;
                return; 
            }
        } else {
            startButtonHoldTime = 0; 
        }
    }
    
    // --- 2. State Handlers (Continuous Logic) ---

    if (gameState == ATTRACT_MODE) {
        RunAttractModeLights(CurrentTime);
        UpdatePlayerDisplay(); 
    }
    else if (gameState == MATCH_MODE) { 
        RunMatchMode(CurrentTime); 
        UpdatePlayerDisplay(); 
    }
    else if (gameState == HIGH_SCORE_CHECK) {
        CheckHighScores(); 
        gameState = GAME_OVER; 
    }
    else if (gameState == BALL_IN_PLAY || gameState == BONUS_COUNT) {
        ProcessSwitches(); 
        RunBallSaveLogic(CurrentTime); 
        HandleArcSurgeCombo(CurrentTime);
        RunBonusLadderChase(CurrentTime);
        RunBonusCountdown(CurrentTime); 
        RunBallSearch(CurrentTime); 
        UpdatePlayerDisplay(); 

        if (!firstHitMade) {
            RPU_SetLampState(LAMP_SAUCER, 1, 0, 500); // Pulse the lamp for skill shot
        } else {
            // This handles turning it off if it was pulsing from ArcSurge but that combo timed out.
        }
    }

    // --- 3. Display Updates (Runs if game is active or waiting) ---
    if (gameState == ATTRACT_MODE || gameState == GAME_OVER || gameState == BALL_IN_PLAY) {
        for (int i = 1; i <= 4; i++) {
            if (i > gNumPlayers) {RPU_SetDisplay(i - 1, 0, true, 1); }
        }
    }
    
    // --- 4. Switch Stack Processing (Input Handler) ---
    byte switchHit;
    while ((switchHit = RPU_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY) {
        
        // --- TILT Logic ---
        if (switchHit == SW_TILT) {
            if (NumTiltWarnings < MaxTiltWarnings) {
                NumTiltWarnings++;
                PlayStockSound(SND_1000_POINTS); 
            } else {
                DrainBall(true); 
                return; 
            }
        }
        
        if (switchHit == SW_SLAM_TILT) {
            DrainBall(true);
            return;
        }

        // --- Coin and Start Button logic ---
        bool isCoinSwitch = (switchHit == SW_COIN_1 || switchHit == SW_COIN_2 || switchHit == SW_COIN_3);
        if (isCoinSwitch && (gameState == ATTRACT_MODE || gameState == GAME_OVER)) {
            int maxCredits = 8;
            if (credits < maxCredits) {
                credits++;
                PlayCreditAddMelody(); 
            }
            RPU_SetCoinLockout(credits >= 8);
        } 
        
        // 1. Start game from Attract/Game Over (Normal Credit/Free Play Start)
        if (switchHit == SW_CREDIT_BUTTON && (gameState == ATTRACT_MODE || gameState == GAME_OVER)) {
             bool freePlay = RPU_ReadByteFromEEProm(ADDR_FREE_PLAY_ADJUSTMENT) == 1;
             if (credits > 0 || freePlay) {
                if (!freePlay) {
                    credits--;
                    RPU_SetCoinLockout(credits >= 8);
                }
                if (gameState == ATTRACT_MODE) PlayGameStartMelody();
                StopAttractModeLights();
                StartGame(1);
                return;
             }
        } 
        
        // 2. Adding Players Logic (While game is active and not max players)
        if (switchHit == SW_CREDIT_BUTTON && gNumPlayers < 4 && gameState == BALL_IN_PLAY) {
            if (startButtonHoldTime == 0) {
                gNumPlayers++;
                PlayGameStartMelody();
            }
        }
    }
}

void StopAttractModeLights() {
    RPU_TurnOffAllLamps(); // Simplest way to ensure a clean slate

    attractPhase = ATTRACT_PHASE_1_CLASSIC_FLOW;
    attractStep = 0;
    gAttractModeTimer = 0;
    gSpinnerAdvancerCount = 0;
    gChaserIndex = 0;
}

void RunAttractModeLights(unsigned long CurrentTime) {
    const unsigned long ATTRACT_PHASE_TIMER_MS = 80;

    RPU_SetLampState(LAMP_SAUCER, 1, 0, 750); // Pulse effect
    
    if (CurrentTime < gAttractModeTimer) {
        return;
    }
    
    gAttractModeTimer = CurrentTime + 80;

    switch (attractPhase) {
        
        case ATTRACT_PHASE_1_CLASSIC_FLOW:
            RPU_SetLampState(SpinnerAdvanceLamps[gSpinnerAdvancerCount % NUM_SPINNER_ADVANCE_LAMPS], 0);
            gSpinnerAdvancerCount++;
            
            if (gSpinnerAdvancerCount >= 44) {
                RPU_SetLampState(LAMP_BONUS_10000, 1);

                RPU_SetLampState(LAMP_BONUS_MULTIPLIER_2X, 1);
                RPU_SetLampState(LAMP_BONUS_MULTIPLIER_3X, 1);
                RPU_SetLampState(LAMP_BONUS_MULTIPLIER_5X, 1);
                RPU_SetLampState(LAMP_SPINNER, 1); // Spinner Lamp
                
                attractPhase = ATTRACT_PHASE_2_ARC_SURGE;
                gAttractModeTimer = CurrentTime + 3000; // Hold the final lights for 3 seconds
                attractStep = 1; // Start Phase 2 at Step 1
                return;
            }
            
            if (gSpinnerAdvancerCount % NUM_SPINNER_ADVANCE_LAMPS == 0) {
                if (gChaserIndex > 0) {
                    RPU_SetLampState(BonusLadderLamps[gChaserIndex - 1], 0);
                }
                RPU_SetLampState(BonusLadderLamps[gChaserIndex], 1);
                gChaserIndex++; // Move to the next bonus ladder light
            }
            
            RPU_SetLampState(SpinnerAdvanceLamps[gSpinnerAdvancerCount % NUM_SPINNER_ADVANCE_LAMPS], 1);
            break;

        case ATTRACT_PHASE_2_ARC_SURGE:
            if (attractStep == 1) {
                StopAttractModeLights(); // Clear lights from previous step
                RPU_SetLampState(LAMP_EXTRA_BALL_LANE, 1, 0, 250); // Flash
                gAttractModeTimer = CurrentTime + 1500;
                attractStep = 2;
            } else if (attractStep == 2) {
                RPU_SetLampState(LAMP_EXTRA_BALL_LANE, 0);
                RPU_SetLampState(LAMP_BONUS_MULTIPLIER_2X, 1, 0, 200);
                RPU_SetLampState(LAMP_BONUS_MULTIPLIER_3X, 1, 0, 200);
                RPU_SetLampState(LAMP_BONUS_MULTIPLIER_5X, 1, 0, 200);
                gAttractModeTimer = CurrentTime + 2000;
                attractStep = 3;
            } else if (attractStep == 3) {
                StopAttractModeLights();
                RPU_SetLampState(LAMP_SAUCER, 1, 0, 250); // Flash
                gAttractModeTimer = CurrentTime + 1500;
                attractStep = 4;
            } else {
                attractPhase = ATTRACT_PHASE_3_WAVE;
                gChaserIndex = 0; // Reset chaser for Phase 3
                gAttractModeTimer = CurrentTime + 10;
            }
            break;

        case ATTRACT_PHASE_3_WAVE:
            gAttractModeTimer = CurrentTime + 50; // Very fast chaser
            
            if (gChaserIndex < NUM_ALL_FEATURE_LAMPS) {
                RPU_SetLampState(AllFeatureLamps[gChaserIndex], 1);
                if (gChaserIndex > 0) {
                    RPU_SetLampState(AllFeatureLamps[gChaserIndex - 1], 0);
                }
                gChaserIndex++;
            } else {
                StopAttractModeLights(); // This will reset all variables including attractPhase
            }
            break;
    }
}

void setup() {
  RPU_init(RPU_Callback_GameLogic);

  highScore = RPU_ReadULFromEEProm(ADDR_HIGH_SCORE, 100000);
  byte ballsPerGameSetting = RPU_ReadByteFromEEProm(ADDR_BALLS_PER_GAME);
  ballsPerGame = (ballsPerGameSetting == 1) ? 5 : 3;

  MaxTiltWarnings = RPU_ReadByteFromEEProm(ADDR_MAX_TILT_WARNINGS);
  if (MaxTiltWarnings > 2) MaxTiltWarnings = 2;
  ExtraBallScoreValue = RPU_ReadULFromEEProm(ADDR_EXTRA_BALL_SCORE, 25000L); 
  SpecialScoreValue = RPU_ReadULFromEEProm(ADDR_SPECIAL_SCORE, 50000L); 
  
  RPU_SetupGameSwitches(NUM_SWITCHES, NUM_PRIORITY_SWITCHES, gameSwitchArray); 
  
  gameState = ATTRACT_MODE;
  credits = 0;

  CheckAndResetSolenoids();
  PlayGameStartMelody();
}

void loop() {
  RPU_loop();
}
