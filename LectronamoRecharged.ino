// LectronamoRecharged.ino
// Main Arduino Sketch for Lectronamo Recharged (RPU Format, Stern MPU-100 Compatible)

#include "RPU.h"
#include "Lectronamo.h"

//================================================================
// I. GLOBAL GAME VARIABLES
//================================================================
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
int gNumPlayers;
bool specialAwardedThisBall;
unsigned long highScore;
int credits;
uint8_t gGameFlags = 0;
unsigned long lastSwitchHitTime = 0;
unsigned long gAttractModeTimer = 0;
int gSpinnerAdvancerCount = 0;
int gChaserIndex = 0;

// --- Attract Mode Lamp Arrays ---

// Attract Mode Spinner Advance Lamps
const byte SpinnerAdvanceLamps[] = {
    LAMP_BONUS_2X_NEXT, LAMP_BONUS_3X_NEXT, LAMP_BONUS_5X_NEXT, LAMP_SPINNER
};
const int NUM_SPINNER_ADVANCE_LAMPS = 4;

// Attract Mode Bonus Ladder Lamps
const byte BonusLadderLamps[] = {
    LAMP_BONUS_1000, LAMP_BONUS_2000, LAMP_BONUS_3000, LAMP_BONUS_4000, 
    LAMP_BONUS_5000, LAMP_BONUS_6000, LAMP_BONUS_7000, LAMP_BONUS_8000, 
    LAMP_BONUS_9000, LAMP_BONUS_10000
};
const int NUM_BONUS_LADDER_LAMPS = 10;

// Attract Mode Wave Lamps
const byte AllFeatureLamps[] = {
    LAMP_BONUS_1000, LAMP_BONUS_2000, LAMP_BONUS_3000, LAMP_BONUS_4000, 
    LAMP_BONUS_5000, LAMP_BONUS_6000, LAMP_BONUS_7000, LAMP_BONUS_8000, 
    LAMP_BONUS_9000, LAMP_BONUS_10000,
    LAMP_BONUS_2X_NEXT, LAMP_BONUS_3X_NEXT, LAMP_BONUS_5X_NEXT,
    LAMP_BONUS_DOUBLE, LAMP_BONUS_TRIPLE, LAMP_BONUS_QUINTUPLE,
    LAMP_SAUCER_EJECT, LAMP_ADVANCE_BONUS_3, LAMP_EXTRA_BALL_LANE, 
    LAMP_SPINNER, LAMP_SHOOT_AGAIN
};
const int NUM_ALL_FEATURE_LAMPS = sizeof(AllFeatureLamps) / sizeof(AllFeatureLamps[0]);

// --- Helper Functions ---
void StopAttractModeLights();
void RunAttractModeLights(unsigned long CurrentTime);

//================================================================
// II. RPU HARDWARE MAPPING
//================================================================
#define NUM_SWITCHES 40 // Total number of switches for the RPU to scan
#define NUM_PRIORITY_SWITCHES 9 // Number of switches with immediate solenoid responses

// This array defines priority switches that fire solenoids instantly in the RPU interrupt.
PlayfieldAndCabinetSwitch gameSwitchArray[NUM_PRIORITY_SWITCHES] = {
    // { Switch ID, Solenoid ID, Hold Time }
    { SW_RIGHT_SLINGSHOT, SOL_RIGHT_SLINGSHOT, 4 },
    { SW_LEFT_SLINGSHOT, SOL_LEFT_SLINGSHOT, 4 },
    { SW_THUMPER_CENTER, SOL_CENTER_THUMPER, 6 },
    { SW_THUMPER_RIGHT, SOL_RIGHT_THUMPER, 6 },
    { SW_THUMPER_LEFT, SOL_LEFT_THUMPER, 6 },
    { SW_OUTHOLE, SOL_NONE, 0 }, // Scanned, but no immediate solenoid
    { SW_SAUCER, SOL_NONE, 0 }, // Scanned, but no immediate solenoid
    { SW_TILT, SOL_NONE, 0 },
    { SW_SLAM_TILT, SOL_NONE, 0 }
};

//================================================================
// III. CORE RPU CALLBACKS & SETUP
//================================================================

void SaveHighScore() {
    if (currentScore > highScore) {
        highScore = currentScore;
        RPU_WriteULToEEProm(ADDR_HIGH_SCORE, highScore);
    }
}
/*
void DrainBall(bool isTilted = false) {
    if (isTilted) { 
        currentBonus = 0; 
    }
    SaveHighScore();
    gameState = BONUS_COUNT;
}
*/
void DrainBall(bool isTilted = false) {
    if (isTilted) {
        currentBonus = 0;
    }
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


//================================================================
// CORE GAME FLOW AND SWITCH DISPATCHER
//================================================================

void AddToPlayerScore(long score) {
    currentScore += score;
}

void PlayStockSound(byte soundID) {
    RPU_PlaySB100(soundID);
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

// Bonus countdown sound using standard solenoid 5 and a final tone.
void PlayBonusCollectSound() {
    PlayStockSound(SND_ADD_BONUS); // Solenoid 5 - Standard bonus chime
    delay(500);
    PlayStockSound(SND_10000_POINTS); // Solenoid 4 - Final chime tone
}

// Extra Ball Award Melody (Celebratory Sequence)
void PlayExtraBallAward() {
    PlayStockSound(SND_10000_POINTS);
    delay(100);
    PlayStockSound(SND_POP_BUMPER);
    delay(100);
    PlayStockSound(SND_10000_POINTS);
}

void RunBonusCountdown(unsigned long CurrentTime) {
    // These static variables manage the state across multiple calls
    static unsigned long lastBonusStepTime = 0;
    static int bonusStepsRemaining = 0;
    static long bonusToAward = 0;
    static long bonusAwarded = 0;
    static int currentBonusLampIndex = NUM_BONUS_LADDER_LAMPS - 1; // Start at 10k (index 9)
    const unsigned long STEP_INTERVAL = 150; // 150ms per bonus step

    if (gameState != BONUS_COUNT) {
        // Reset state when leaving BONUS_COUNT
        bonusStepsRemaining = 0;
        return;
    }

    // Initialize state on first entry to BONUS_COUNT
    if (bonusStepsRemaining == 0 && lastBonusStepTime == 0) {
        // Calculate total bonus to award
        bonusToAward = (long)currentBonus * bonusMultiplier;
        // Calculate the maximum number of 1000-point steps to count down
        bonusStepsRemaining = currentBonus / 1000; 
        bonusAwarded = 0;
        currentBonusLampIndex = (currentBonus / 1000) - 1; // Index 0 = 1k, Index 9 = 10k, Index 18 = 19k
        
        // Ensure the initial lamp state is correct (all lit up to the current bonus)
        for (int i = 0; i <= currentBonusLampIndex && i < NUM_BONUS_LADDER_LAMPS; i++) {
             RPU_SetLampState(BonusLadderLamps[i], 1);
        }
        
        lastBonusStepTime = CurrentTime; 
    }

    if (CurrentTime < lastBonusStepTime + STEP_INTERVAL) {
        return; // Wait for the next step interval
    }
    
    // Check if the total score has been awarded
    if (bonusStepsRemaining <= 0) {
        // COUNTDOWN COMPLETE - Award any remainder and eject.
        if (bonusAwarded < bonusToAward) {
            AddToPlayerScore(bonusToAward - bonusAwarded);
        }
        gameState = BALL_IN_PLAY; 
        FireSolenoid(SOL_OUTHOLE, 150); // Eject ball after countdown is finished
        currentBonus = 1000; // Reset Bonus to 1000 for next ball
        lastBonusStepTime = 0; // Reset timer for next countdown
        return;
    }

    // 1. Visually turn off the corresponding lamp
    if (currentBonusLampIndex >= 0 && currentBonusLampIndex < NUM_BONUS_LADDER_LAMPS) {
         RPU_SetLampState(BonusLadderLamps[currentBonusLampIndex], 0);
    }
    
    // 2. Award 1000 points (using the multiplier)
    long scoreStep = 1000L * bonusMultiplier;
    
    if (bonusToAward - bonusAwarded < scoreStep) {
        scoreStep = bonusToAward - bonusAwarded; 
    }
    
    AddToPlayerScore(scoreStep); 
    bonusAwarded += scoreStep;
    PlayStockSound(SND_100_POINTS); // Play a scoring sound for the step
    
    // 3. Decrement counters and move to the next lamp
    bonusStepsRemaining--;
    currentBonusLampIndex--;
    
    lastBonusStepTime = CurrentTime;
}

void RunBonusLadderChase(unsigned long CurrentTime) {
    static unsigned long lastChaseTime = 0;
    const unsigned long CHASE_INTERVAL = 50; // Fast visual update

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

    if (gameState == BALL_IN_PLAY) {
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
    PlayStockSound(SND_100_POINTS); // Play 100-point sound on every spinner hit
    
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

    RPU_SetDisplay(0, spinnerHitCount, true); // Use Display 0 (Player 1) for status
}

//================================================================
// CORE RULESET HANDLERS (Final Logic)
//================================================================

// Clears all 6 multiplier lamps.
void ClearAllMultiplierLamps() {
    // Set 1 (Current Status - Bonus Ladder)
    RPU_SetLampState(LAMP_BONUS_DOUBLE, 0, 0, 0);
    RPU_SetLampState(LAMP_BONUS_TRIPLE, 0, 0, 0);
    RPU_SetLampState(LAMP_BONUS_QUINTUPLE, 0, 0, 0);
    // Set 2 (Next Target - 3-Bank Area)
    RPU_SetLampState(LAMP_BONUS_2X_NEXT, 0, 0, 0);
    RPU_SetLampState(LAMP_BONUS_3X_NEXT, 0, 0, 0);
    RPU_SetLampState(LAMP_BONUS_5X_NEXT, 0, 0, 0);
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
        RPU_SetLampState(LAMP_BONUS_DOUBLE, 1, 0, 0); 
        RPU_SetLampState(LAMP_BONUS_3X_NEXT, 1, 0, 0);
    } else if (threeBankCompleteCount == 2) {
        bonusMultiplier = 3;
        RPU_SetLampState(LAMP_BONUS_TRIPLE, 1, 0, 0); 
        RPU_SetLampState(LAMP_BONUS_5X_NEXT, 1, 0, 0);
    } else if (threeBankCompleteCount >= 3) {
        bonusMultiplier = 5;
        RPU_SetLampState(LAMP_BONUS_QUINTUPLE, 1, 0, 0); 
    }
}

// Handles 5-Bank completion and Extra Ball/Special logic.
void Handle5BankCompletion() {
    AddToPlayerScore(SCORE_5BANK_COMPLETION);
    FireSolenoid(SOL_DROP_TARGET_5BANK_RESET, 100); 
    
    fiveBankCompleteCount++;
    for (int i = 0; i < 5; i++) {
        fiveTargetsDown[i] = false;
    }

    // --- Action B: Extra Ball Check (SW 26) ---
    byte extraBallSetting = RPU_ReadByteFromEEProm(ADDR_EXTRA_BALL_BYPASS);
    bool awardExtraBall = (extraBallSetting != 0); // If setting is not 0 (Bypass)

    if (fiveBankCompleteCount == 2) {
        if (awardExtraBall) { // Check operator adjustment SW 26
            RPU_SetLampState(LAMP_EXTRA_BALL_LANE, 1, 0, 0); // Lite Extra Ball Lane
            extraBallLit = true;
        }
    } else if (fiveBankCompleteCount >= 3) {
        RPU_SetLampState(LAMP_BONUS_QUINTUPLE, 1, 0, 500); // Placeholder Flash for Special
        PlayExtraBallAward(); // Play sound when Special condition is met
    }
}

void CollectBonus() {
    // This function is now a placeholder. The logic has been moved to DrainBall and RunBonusCountdown.
    // The primary purpose is to set the state and let the main loop handle the countdown.
    PlayBonusCollectSound(); // Play the initial sound
    ClearAllMultiplierLamps(); // Reset multiplier lamps for the next ball
    gameState = BONUS_COUNT; // Set the state to start the countdown
}
void ProcessSwitches() {
    byte switchHit;
    while ((switchHit = RPU_PullFirstFromSwitchStack()) != NO_SWITCH_HIT) {
        // Update the Ball Search Timer on any switch activity
        // --- Action A: Reset Ball Search Timer ---
        unsigned long CurrentTime = millis();
        lastSwitchHitTime = CurrentTime; // Update the Ball Search Timer
        lastSwitchHitTime = millis();

        switch (switchHit) {
            case SW_OUTHOLE:
                DrainBall();
                break;

            case SW_KICKER: // Side Lane Kicker
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

            case SW_SPIN_TARGET:
                HandleSpinnerHit();
                break;

            case SW_THUMPER_CENTER:
            case SW_THUMPER_RIGHT:
            case SW_THUMPER_LEFT:
                AddToPlayerScore(SCORE_POP_BUMPER);
                PlayStockSound(SND_POP_BUMPER);
                break;

            case SW_RIGHT_RETURN_LANE:
                AddToPlayerScore(100L);
                break;
            
            case SW_ADV_BONUS_1000: // Arc Surge Target 1
                // Logic is handled inside HandleArcSurgeCombo()
                break;

            case SW_SAUCER: // Saucer / Eject Pocket
                HandleSkillShot(switchHit); // Handles both Skill Shot and normal saucer logic
                break;

            case SW_STANDUP_TARGET: // 5000 pts and 1 bonus advance
                gGameFlags |= FLAG_SIDE_LANE_LIT; // Set flag for saucer interaction
                AddToPlayerScore(5000L); 
                currentBonus += 1000;
                PlayStockSound(SND_1000_POINTS);
                break;

            case SW_RIGHT_SLINGSHOT: // SW 38
            case SW_LEFT_SLINGSHOT:  // SW 39
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

            case SW_LEFT_RETURN_LANE:
                if (gGameFlags & FLAG_LEFT_RETURN_LANE_LIT) {
                    AddToPlayerScore(9000L);
                    gGameFlags &= ~FLAG_LEFT_RETURN_LANE_LIT; // Turn off the light
                    PlayStockSound(SND_10000_POINTS); // ADDED: Very high-value sound
                } else {
                    AddToPlayerScore(SCORE_SPINNER_BASE); 
                    PlayStockSound(SND_100_POINTS); // ADDED: Standard 100 pt sound
                }
                break;
        }
    }

    if (threeTargetsDown[0] && threeTargetsDown[1] && threeTargetsDown[2]) {
        Handle3BankCompletion();
    }
    if (fiveTargetsDown[0] && fiveTargetsDown[1] && fiveTargetsDown[2] && fiveTargetsDown[3] && fiveTargetsDown[4]) {
        Handle5BankCompletion();
    }
}

//================================================================
// CUSTOM RULESET IMPLEMENTATION (Lectronamo: Recharged)
//================================================================

// Handles Skill Shot and Saucer Eject logic.
void HandleSkillShot(byte switchHit) {
    if (gameState == BALL_IN_PLAY && !firstHitMade) {
        if (switchHit == SW_SAUCER) {
            AddToPlayerScore(SCORE_SKILL_SHOT);
            PlayStockSound(SND_1000_POINTS); // Use 1000 pt sound for base skill shot
        }
        firstHitMade = true; // Prevents subsequent Skill Shots
    }
    
    if (switchHit == SW_SAUCER) {
        if (gGameFlags & FLAG_SIDE_LANE_LIT) {
            AddToPlayerScore(5000L); // 5,000 points
            currentBonus += 3000;    // 3 bonus advances
            PlayStockSound(SND_10000_POINTS); // Use high-value sound
        } else {
            AddToPlayerScore(500L);  // 500 points
            currentBonus += 1000;    // 1 bonus advance
            PlayStockSound(SND_1000_POINTS); // Base sound
        }
        FireSolenoid(SOL_SAUCER, 50); // Fire Solenoid 14 briefly

        // --- Action C: Saucer Light Persistence Check (SW 14) ---
        byte persistenceSetting = RPU_ReadByteFromEEProm(ADDR_SAUCER_LIGHT_PERSISTENCE); 
        if (persistenceSetting == 0) { // If setting is 'Goes OFF'
            RPU_SetLampState(LAMP_SAUCER_EJECT, 0); // Turn off the lamp
        }
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
    if (RPU_ReadSingleSwitchState(SW_RIGHT_RETURN_LANE) && !(gGameFlags & FLAG_ARC_SURGE_ACTIVE)) {
        gGameFlags |= FLAG_ARC_SURGE_ACTIVE;
        arcSurgeTimerStart = CurrentTime;
        RPU_SetLampState(LAMP_SAUCER_EJECT, 1, 0, 500); // Pulse target
    }

    if (gGameFlags & FLAG_ARC_SURGE_ACTIVE) {
        if (CurrentTime > arcSurgeTimerStart + TIME_ARC_SURGE_COMBO_MS) {
            gGameFlags &= ~FLAG_ARC_SURGE_ACTIVE; // Turn off flag
            RPU_SetLampState(LAMP_SAUCER_EJECT, 0); 
        } else {
            if (RPU_ReadSingleSwitchState(SW_ADV_BONUS_1000)) { // Target 1 Hit
                gGameFlags |= FLAG_ARC_SURGE_T1_HIT;
                AddToPlayerScore(SCORE_ARC_SURGE_T1);
                PlayStockSound(SND_1000_POINTS); 
            }
            if (RPU_ReadSingleSwitchState(SW_SAUCER) && (gGameFlags & FLAG_ARC_SURGE_T1_HIT)) { // Super Target Hit
                AddToPlayerScore(SCORE_ARC_SURGE_SUPER);
                PlayStockSound(SND_10000_POINTS); // Use high-value sound for combo completion
                
                gGameFlags &= ~(FLAG_ARC_SURGE_ACTIVE | FLAG_ARC_SURGE_T1_HIT);
                RPU_SetLampState(LAMP_SAUCER_EJECT, 0); 
                FireSolenoid(SOL_SAUCER, 50); // Fire Solenoid 14
            }
        }
    }
}

void StartGame(byte numPlayers) {
    gameState = BALL_IN_PLAY; 
    currentScore = 0;
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
    
    PlayGameStartMelody();
    
}




void RPU_Callback_GameLogic() {
    static unsigned long startButtonHoldTime = 0;
    const unsigned long HOLD_TIME_TO_RESET_MS = 2000; // Hold for 2 seconds

    if (gameState == BALL_IN_PLAY) {
    if (gameState == BALL_IN_PLAY && ball > 1) {
        if (RPU_ReadSingleSwitchState(SW_CREDIT_BUTTON)) {
            if (startButtonHoldTime == 0) startButtonHoldTime = millis();
            else if (millis() > startButtonHoldTime + HOLD_TIME_TO_RESET_MS) {
                StartGame(1); // Restart the game
                startButtonHoldTime = 0;
                return; // Exit immediately
            }
        } else {
            startButtonHoldTime = 0; // Reset timer if button is released
        }
    }
    
    if (gameState == ATTRACT_MODE) {
        RunAttractModeLights(millis());
    } else if (gameState == BALL_IN_PLAY || gameState == BONUS_COUNT) {
        unsigned long CurrentTime = millis();
        
        ProcessSwitches(); 
        RunBallSaveLogic(CurrentTime);
        HandleArcSurgeCombo(CurrentTime);
        RunBonusCountdown(CurrentTime);
        RunBonusLadderChase(CurrentTime); 
        RunBallSearch(CurrentTime);
    }

    if (gameState == ATTRACT_MODE || gameState == GAME_OVER || gameState == BALL_IN_PLAY) {
        for (int i = 1; i <= 4; i++) {
            if (i > gNumPlayers) {
                RPU_SetDisplay(i - 1, 0, true); 
            }
        }
    }

    byte switchHit; // Use NO_SWITCH_HIT from RPU library if available, otherwise 0xFF
    while ((switchHit = RPU_PullFirstFromSwitchStack()) != NO_SWITCH_HIT) {
        bool isCoinSwitch = (switchHit == SW_COIN_1 || switchHit == SW_COIN_2 || switchHit == SW_COIN_3);

        if (isCoinSwitch && (gameState == ATTRACT_MODE || gameState == GAME_OVER)) {
            int maxCredits = 8; // Placeholder

            if (credits < maxCredits) {
                credits++;
                PlayCreditAddMelody();
            }
        }

        if (switchHit == SW_CREDIT_BUTTON && credits > 0 && (gameState == ATTRACT_MODE || gameState == GAME_OVER)) {
            credits--;
            StopAttractModeLights();
            StartGame(1); // Start a 1-player game
        } else if (switchHit == SW_CREDIT_BUTTON && (gameState == ATTRACT_MODE || gameState == GAME_OVER)) {
            byte freePlaySetting = RPU_ReadByteFromEEProm(ADDR_FREE_PLAY_ADJUSTMENT);
            if (freePlaySetting == 1) {
                StopAttractModeLights();
                StartGame(1);
            }
        } else if (switchHit == SW_CREDIT_BUTTON && gNumPlayers < 4 && gameState == BALL_IN_PLAY) {
            gNumPlayers++;
            PlayGameStartMelody();
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

    RPU_SetLampState(LAMP_SAUCER_EJECT, 1, 0, 750); // Pulse effect
    
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
                
                RPU_SetLampState(LAMP_BONUS_DOUBLE, 1);
                RPU_SetLampState(LAMP_BONUS_TRIPLE, 1);
                RPU_SetLampState(LAMP_BONUS_QUINTUPLE, 1);
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
                RPU_SetLampState(LAMP_BONUS_DOUBLE, 1, 0, 200);
                RPU_SetLampState(LAMP_BONUS_TRIPLE, 1, 0, 200);
                RPU_SetLampState(LAMP_BONUS_QUINTUPLE, 1, 0, 200);
                gAttractModeTimer = CurrentTime + 2000;
                attractStep = 3;
            } else if (attractStep == 3) {
                StopAttractModeLights();
                RPU_SetLampState(LAMP_SAUCER_EJECT, 1, 0, 250); // Flash
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

  highScore = RPU_ReadULFromEEProm(ADDR_HIGH_SCORE, 100000); // Load high score with a default of 100,000
  byte ballsPerGameSetting = RPU_ReadByteFromEEProm(ADDR_BALLS_PER_GAME);
  ballsPerGame = (ballsPerGameSetting == 1) ? 5 : 3;
  
  RPU_SetupGameSwitches(NUM_SWITCHES, NUM_PRIORITY_SWITCHES, gameSwitchArray); 
  
  gameState = ATTRACT_MODE; 
  credits = 0;

  CheckAndResetSolenoids(); // Perform conditional ejects and resets after RPU is ready
}

void loop() {
  RPU_loop();
}
