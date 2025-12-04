// LectronamoRecharged.ino
// Main Arduino Sketch for Lectronamo Recharged (RPU Format, Stern MPU-100 Compatible)

#include "RPU.h"
#include "Lectronamo.h"

//================================================================
// I. GLOBAL GAME VARIABLES
//================================================================
// These variables are declared as 'extern' in Lectronamo.h
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
uint8_t gGameFlags = 0; // Bitmask for game state flags
unsigned long gAttractModeTimer = 0;
int gSpinnerAdvancerCount = 0; // For Attract Phase 1
int gChaserIndex = 0;          // For Attract Phase 1 & 3

// --- Attract Mode Lamp Arrays ---

// Phase 1: Spinner Advance Lamps (Assumed from manual/design, using available pins)
const byte SpinnerAdvanceLamps[] = {
    LAMP_BONUS_2X_NEXT, LAMP_BONUS_3X_NEXT, LAMP_BONUS_5X_NEXT, LAMP_SPINNER
};
const int NUM_SPINNER_ADVANCE_LAMPS = 4;

// Phase 1 & 2: Bonus Ladder Lamps (for the classic chase)
const byte BonusLadderLamps[] = {
    LAMP_BONUS_1000, LAMP_BONUS_2000, LAMP_BONUS_3000, LAMP_BONUS_4000, 
    LAMP_BONUS_5000, LAMP_BONUS_6000, LAMP_BONUS_7000, LAMP_BONUS_8000, 
    LAMP_BONUS_9000, LAMP_BONUS_10000
};
const int NUM_BONUS_LADDER_LAMPS = 10;

// All Feature Lamps (Used for Phase 3: The Recharged Wave)
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

void DrainBall(bool isTilted = false) {
    if (isTilted) { 
        currentBonus = 0; 
    }
    SaveHighScore();
    gameState = BONUS_COUNT;
}

//================================================================
// CORE GAME FLOW AND SWITCH DISPATCHER
//================================================================

// --- Placeholder/Helper functions for ProcessSwitches ---
void AddToPlayerScore(long score) {
    currentScore += score;
}

void PlayStockSound(byte soundID) {
    // For the SB-100, we use a dedicated RPU function that takes a bitmask.
    RPU_PlaySB100(soundID);
}

void RunBonusLadderChase(unsigned long CurrentTime) {
    static unsigned long lastChaseTime = 0;
    const unsigned long CHASE_INTERVAL = 50; // Fast visual update

    if ((gGameFlags & FLAG_ARC_SURGE_ACTIVE) && CurrentTime > lastChaseTime + CHASE_INTERVAL) {
        // Clear previous light
        RPU_SetLampState(BonusLadderLamps[gChaserIndex], 0);

        // Advance index and wrap around 0-9
        gChaserIndex = (gChaserIndex + 1) % NUM_BONUS_LADDER_LAMPS;

        // Set current light ON
        RPU_SetLampState(BonusLadderLamps[gChaserIndex], 1);

        lastChaseTime = CurrentTime;
    } else if (!(gGameFlags & FLAG_ARC_SURGE_ACTIVE)) {
        // Ensure all lights are off when the combo is not running
        RPU_SetLampState(BonusLadderLamps[gChaserIndex], 0);
    }
}

void FireSolenoid(byte sol, int holdTime) {
    RPU_PushToSolenoidStack(sol, holdTime);
}

void HandleSpinnerHit() {
    spinnerHitCount++;
    PlayStockSound(SND_100_POINTS); // Play 100-point sound on every spinner hit
    
    long score = SCORE_SPINNER_BASE; 

    // 1. Check if the Spinner is "Lit" (Bonus >= 10K)
    if (currentBonus >= 10000) {
        score = SCORE_SPINNER_LIT; // 1,000 points
        RPU_SetLampState(LAMP_SPINNER, 1); // Turn ON Spinner Lit Lamp
    } else {
        RPU_SetLampState(LAMP_SPINNER, 0); // Turn OFF Spinner Lit Lamp
    }
    AddToPlayerScore(score); 

    // 2. Bonus Advancement: Every 4th spin advances the bonus by 1,000.
    if (spinnerHitCount % 4 == 0) {
        currentBonus += 1000; 
        if (currentBonus > 19000) {
            currentBonus = 19000;
        }
    }

    // 3. Custom: Momentarily display the spinner hit count
    RPU_SetDisplay(0, spinnerHitCount, true); // Use Display 0 (Player 1) for status
}

//================================================================
// CORE RULESET HANDLERS (Final Logic)
//================================================================

// --- Utility function to clear all 6 multiplier lamps ---
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

// --- Handles 3-Bank Completion and Bonus Multiplier Logic ---
void Handle3BankCompletion() {
    // 1. Award base score (6,000 points) and reset targets.
    AddToPlayerScore(SCORE_3BANK_COMPLETION); 
    FireSolenoid(SOL_DROP_TARGET_3BANK_RESET, 100); 
    
    // 2. Advance completion count and clear tracking flags.
    threeBankCompleteCount++;
    for (int i = 0; i < 3; i++) { threeTargetsDown[i] = false; }

    // 3. Set Multiplier Lamps (Dual Set Logic)
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

// --- Handles 5-Bank Completion and Extra Ball/Special Logic ---
void Handle5BankCompletion() {
    // 1. Award base score (10,000 points) and reset targets.
    AddToPlayerScore(SCORE_5BANK_COMPLETION); 
    FireSolenoid(SOL_DROP_TARGET_5BANK_RESET, 100); 
    
    // 2. Advance completion count and clear tracking flags.
    fiveBankCompleteCount++;
    for (int i = 0; i < 5; i++) { fiveTargetsDown[i] = false; }

    // 3. Set Extra Ball and Special status.
    if (fiveBankCompleteCount == 2) {
        RPU_SetLampState(LAMP_EXTRA_BALL_LANE, 1, 0, 0); // Lite Extra Ball Lane (J3 Pin 18)
        extraBallLit = true;
    } else if (fiveBankCompleteCount >= 3) {
        RPU_SetLampState(LAMP_BONUS_QUINTUPLE, 1, 0, 500); // Placeholder Flash for Special
    }
}

// --- Handles Bonus Score Calculation and Display Countdown ---
void CollectBonus() {
    // 1. Award Knocker if Special or High Score Replay was earned.
    // NOTE: In a complete game, award check logic would run here.
    // If (Award Logic Here) { FireSolenoid(SOL_KNOCKER, 50); }

    // 2. Calculate the final score to be awarded
    long scoreAward = (long)currentBonus * bonusMultiplier;
    
    // 3. Determine the display countdown method (MPU Switch 24)
    byte countdownMethod = RPU_ReadByteFromEEProm(ADDR_BONUS_COUNTDOWN_METHOD);
    bool countdownBy1000Steps = (countdownMethod == 1); // 1,000 Steps is MPU SW 24 ON

    // NOTE: The actual display countdown routine is complex. For now, we award instantly.
    AddToPlayerScore(scoreAward);
    
    // 4. Reset Bonus state
    currentBonus = 1000;
    ClearAllMultiplierLamps(); // Turn off all 6 multiplier lamps
    
    // 5. Fire Outhole Kicker
    FireSolenoid(SOL_OUTHOLE, 150); // Fire Outhole Kicker (Sol 10)
}

// --- Handles all switch closures and dispatches to specific handlers ---
void ProcessSwitches() {
    byte switchHit;
    while ((switchHit = RPU_PullFirstFromSwitchStack()) != NO_SWITCH_HIT) {
        // The RPU OS handles the Credit Button (SW 6) and Tilt (SW 7) before this.
        switch (switchHit) {
            // --- Ball Drain / Bonus Collection ---
            case SW_OUTHOLE:
                CollectBonus(); 
                DrainBall(); // Calls the function already defined in your code
                break;

            case SW_KICKER: // Side Lane Kicker
                CollectBonus();
                FireSolenoid(SOL_KICKER, 10); 
                break;
            
            // --- Drop Target Hits (Award base score, then check for completion) ---
            case SW_TARGET_1_3BANK: threeTargetsDown[0] = true; AddToPlayerScore(SCORE_DROP_TARGET_BASE); PlayStockSound(SND_100_POINTS); break;
            case SW_TARGET_2_3BANK: threeTargetsDown[1] = true; AddToPlayerScore(SCORE_DROP_TARGET_BASE); PlayStockSound(SND_100_POINTS); break;
            case SW_TARGET_3_3BANK: threeTargetsDown[2] = true; AddToPlayerScore(SCORE_DROP_TARGET_BASE); PlayStockSound(SND_100_POINTS); break;

            case SW_TARGET_1_5BANK: fiveTargetsDown[0] = true; AddToPlayerScore(SCORE_DROP_TARGET_BASE); PlayStockSound(SND_100_POINTS); break;
            case SW_TARGET_2_5BANK: fiveTargetsDown[1] = true; AddToPlayerScore(SCORE_DROP_TARGET_BASE); PlayStockSound(SND_100_POINTS); break;
            case SW_TARGET_3_5BANK: fiveTargetsDown[2] = true; AddToPlayerScore(SCORE_DROP_TARGET_BASE); PlayStockSound(SND_100_POINTS); break;
            case SW_TARGET_4_5BANK: fiveTargetsDown[3] = true; AddToPlayerScore(SCORE_DROP_TARGET_BASE); PlayStockSound(SND_100_POINTS); break;
            case SW_TARGET_5_5BANK: fiveTargetsDown[4] = true; AddToPlayerScore(SCORE_DROP_TARGET_BASE); PlayStockSound(SND_100_POINTS); break;

            // --- Bonus Advance / Feature Triggers ---
            case SW_SPIN_TARGET:
                HandleSpinnerHit();
                break;

            // --- Thumper-Bumpers (Pop Bumpers) ---
            case SW_THUMPER_CENTER:
            case SW_THUMPER_RIGHT:
            case SW_THUMPER_LEFT:
                // The solenoid firing is handled by the RPU's priority switch mapping in gameSwitchArray.
                // We just need to add score and sound.
                AddToPlayerScore(SCORE_POP_BUMPER);
                PlayStockSound(SND_POP_BUMPER);
                break;

            case SW_RIGHT_RETURN_LANE:
                // This switch activates the Arc Surge Combo
                break;
            
            case SW_ADV_BONUS_1000: // Arc Surge Target 1
                // Logic is handled inside HandleArcSurgeCombo()
                break;

            case SW_SAUCER: // Saucer / Eject Pocket
                HandleSkillShot(switchHit); // Handles both Skill Shot and normal saucer logic
                break;

            // --- Simple Scoring ---
            case SW_STANDUP_TARGET: // 5000 pts and 1 bonus advance
                gGameFlags |= FLAG_SIDE_LANE_LIT; // Set flag for saucer interaction
                AddToPlayerScore(5000L); 
                currentBonus += 1000;
                PlayStockSound(SND_1000_POINTS);
                break;

            // --- Slingshot Logic (100 points, NO bonus advance) ---
            case SW_RIGHT_SLINGSHOT: // SW 38
            case SW_LEFT_SLINGSHOT:  // SW 39
                AddToPlayerScore(100L); // Corrected to 100 points
                PlayStockSound(SND_100_POINTS); // Use the 100 pt sound
                break;
            
            // --- Rebound Rubbers Logic (SW 18 - 300 points + 1 Bonus Advance) ---
            case SW_ADV_BONUS_300: // SW 18 is the 300 Advance Bonus switch (Rebound Rubber)
                AddToPlayerScore(300L); // 300 points
                currentBonus += 1000;    // 1 bonus step advance
                PlayStockSound(SND_100_POINTS); // Use the 100 pt sound (chirp)
                break;

            // --- 10 Point Switches ---
            case SW_SCORE_10: // SW 19
                AddToPlayerScore(10L);
                PlayStockSound(SND_10_POINTS); // Use the 10 pt sound (Solenoid 1)
                break;

            // --- Outlane Logic (3,000 points + 3 Bonus Advances) ---
            case SW_RIGHT_OUTLANE:
            case SW_LEFT_OUTLANE:
                AddToPlayerScore(SCORE_OUTLANE); // 3,000 points
                currentBonus += 3000;            // 3 bonus advances
                break;

            // --- Rollover Button & Left Return Lane Logic ---
            case SW_ROLLOVER_BUTTON:
                AddToPlayerScore(SCORE_SPINNER_BASE); // 100 points
                gGameFlags |= FLAG_LEFT_RETURN_LANE_LIT; // Lite the Left Return Lane
                break;

            case SW_LEFT_RETURN_LANE:
                if (gGameFlags & FLAG_LEFT_RETURN_LANE_LIT) {
                    AddToPlayerScore(9000L);
                    gGameFlags &= ~FLAG_LEFT_RETURN_LANE_LIT; // Turn off the light
                } else {
                    AddToPlayerScore(SCORE_SPINNER_BASE); 
                }
                break;
        }
    }

    // --- Post-Switch Completion Checks ---
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

// --- Handles Skill Shot and Saucer Eject Logic ---
void HandleSkillShot(byte switchHit) {
    // Check if Skill Shot is possible (e.g., first hit of the ball)
    if (gameState == BALL_IN_PLAY && !firstHitMade) {
        // If the switch hit is the Saucer, it's the Skill Shot.
        if (switchHit == SW_SAUCER) {
            AddToPlayerScore(SCORE_SKILL_SHOT);
            // Optionally play a sound effect here
        }
        firstHitMade = true; // Prevents subsequent Skill Shots
    }
    
    // Now handle the Eject Pocket score based on the manual's rules
    if (switchHit == SW_SAUCER) {
        // Check if stationary target was hit (FLAG_SIDE_LANE_LIT flag is set by stationary target hit logic)
        // For POC simplicity, we award Super value if the flag is set:
        if (gGameFlags & FLAG_SIDE_LANE_LIT) {
            AddToPlayerScore(5000L); // 5,000 points
            currentBonus += 3000;    // 3 bonus advances
        } else {
            AddToPlayerScore(500L);  // 500 points
            currentBonus += 1000;    // 1 bonus advance
        }
    }
}

// --- Manages Ball Save Timer and Eject Logic ---
void RunBallSaveLogic(unsigned long CurrentTime) {
    if (isBallSaveActive) {
        RPU_SetLampState(LAMP_SHOOT_AGAIN, 1, 0, 500); // Visual cue (Pulse)
        
        if (CurrentTime > ballSaveStartTime + TIME_BALL_SAVE_DURATION_MS) {
            isBallSaveActive = false;
            RPU_SetLampState(LAMP_SHOOT_AGAIN, 0);
        } else {
            // Check if Outhole is hit while active
            // Note: This requires a way to check for a switch hit outside the main loop,
            // or for ProcessSwitches to handle this state.
        }
    }
}

// --- Arc Surge Combo Activation and Timer Logic ---
void HandleArcSurgeCombo(unsigned long CurrentTime) {
    // 1. Activation Check (Triggered by Right Inlane - SW 2)
    if (RPU_ReadSingleSwitchState(SW_RIGHT_RETURN_LANE) && !(gGameFlags & FLAG_ARC_SURGE_ACTIVE)) {
        gGameFlags |= FLAG_ARC_SURGE_ACTIVE;
        arcSurgeTimerStart = CurrentTime;
        // Start dual visual cues: Pulse the target and start the bonus ladder chase
        RPU_SetLampState(LAMP_SAUCER_EJECT, 1, 0, 500); // Pulse target
    }

    // 2. Timer and Visual Logic
    if (gGameFlags & FLAG_ARC_SURGE_ACTIVE) {
        if (CurrentTime > arcSurgeTimerStart + TIME_ARC_SURGE_COMBO_MS) {
            // Timer expired
            gGameFlags &= ~FLAG_ARC_SURGE_ACTIVE; // Turn off flag
            RPU_SetLampState(LAMP_SAUCER_EJECT, 0); 
        } else {
            // Scoring Logic (T1 must be hit, then Super Target)
            if (RPU_ReadSingleSwitchState(SW_ADV_BONUS_1000)) { // Target 1 Hit
                gGameFlags |= FLAG_ARC_SURGE_T1_HIT;
                AddToPlayerScore(SCORE_ARC_SURGE_T1);
            }
            if (RPU_ReadSingleSwitchState(SW_SAUCER) && (gGameFlags & FLAG_ARC_SURGE_T1_HIT)) { // Super Target Hit
                AddToPlayerScore(SCORE_ARC_SURGE_SUPER);
                gGameFlags &= ~(FLAG_ARC_SURGE_ACTIVE | FLAG_ARC_SURGE_T1_HIT);
                RPU_SetLampState(LAMP_SAUCER_EJECT, 0); 
            }
        }
    }
}

void RPU_Callback_GameLogic() {
    // This function is called repeatedly by RPU_loop()
    // It replaces the old HandleSwitches() and GameFlowUpdate() methods.
    
    if (gameState == ATTRACT_MODE) {
        RunAttractModeLights(millis());
    } else if (gameState == BALL_IN_PLAY || gameState == BONUS_COUNT) {
        unsigned long CurrentTime = millis();
        
        // 1. Process all switch hits since the last loop
        ProcessSwitches(); 
        
        // 2. Run continuous custom timers and logic
        RunBallSaveLogic(CurrentTime);
        HandleArcSurgeCombo(CurrentTime);
        // 3. RUN THE FINAL ARC SURGE VISUAL LOGIC
        RunBonusLadderChase(CurrentTime);
    }

    byte switchHit; // Use NO_SWITCH_HIT from RPU library if available, otherwise 0xFF
    while ((switchHit = RPU_PullFirstFromSwitchStack()) != NO_SWITCH_HIT) {
        // --- Coin Switch Logic ---
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
                // RPU_PlaySound(SND_COIN_IN); // Example
            }
        }

        // --- Start Button Logic ---
        if (switchHit == SW_CREDIT_BUTTON && credits > 0 && (gameState == ATTRACT_MODE || gameState == GAME_OVER)) {
            // Start a new game
            credits--;
            StopAttractModeLights();
            gameState = GAME_START;
            // RPU_PlaySound(SND_GAME_START); // Example
            return; // Exit to allow game start logic to run on the next loop
        }

    }
    // Add main game flow logic (attract mode, game over sequences, etc.) here
}

void StopAttractModeLights() {
    // Turn off all feature lamps used in the attract show
    RPU_TurnOffAllLamps(); // Simplest way to ensure a clean slate

    // Reset state variables
    attractPhase = ATTRACT_PHASE_1_CLASSIC_FLOW;
    attractStep = 0;
    gAttractModeTimer = 0;
    gSpinnerAdvancerCount = 0;
    gChaserIndex = 0;
}

void RunAttractModeLights(unsigned long CurrentTime) {
    // This function implements the 3-phase light show logic
    const unsigned long ATTRACT_PHASE_TIMER_MS = 80;

    // 1. Run the continuous Arc Pulse Flares (pulsing throughout all phases)
    RPU_SetLampState(LAMP_SAUCER_EJECT, 1, 0, 750); // Pulse effect
    
    // We only advance the phase logic on timer expiry
    if (CurrentTime < gAttractModeTimer) {
        return;
    }
    
    // Reset timer for the next step (80ms base interval)
    gAttractModeTimer = CurrentTime + 80;

    switch (attractPhase) {
        
        case ATTRACT_PHASE_1_CLASSIC_FLOW:
            // Phase 1 Logic: Spinner Advance Chase (4 lamps) and Bonus Ladder Progression (11 steps)
            
            // P1.1 - Turn OFF the previous Spinner lamp
            RPU_SetLampState(SpinnerAdvanceLamps[gSpinnerAdvancerCount % NUM_SPINNER_ADVANCE_LAMPS], 0);
            
            // P1.2 - Advance the overall counter (runs 44 times total)
            gSpinnerAdvancerCount++;
            
            // P1.3 - Check for completion of the 11-step sequence
            if (gSpinnerAdvancerCount >= 44) {
                // P1.4 - End of Phase 1 - Set up Reverse Chase (or transition to Phase 2 for simplicity)
                RPU_SetLampState(LAMP_BONUS_10000, 1);
                
                // Light the Final Hold lamps (Bonus Ladder Multipliers)
                RPU_SetLampState(LAMP_BONUS_DOUBLE, 1);
                RPU_SetLampState(LAMP_BONUS_TRIPLE, 1);
                RPU_SetLampState(LAMP_BONUS_QUINTUPLE, 1);
                RPU_SetLampState(LAMP_SPINNER, 1); // Spinner Lamp
                
                attractPhase = ATTRACT_PHASE_2_ARC_SURGE;
                gAttractModeTimer = CurrentTime + 3000; // Hold the final lights for 3 seconds
                attractStep = 1; // Start Phase 2 at Step 1
                return;
            }
            
            // P1.5 - Advance Bonus Ladder every 4th step
            if (gSpinnerAdvancerCount % NUM_SPINNER_ADVANCE_LAMPS == 0) {
                // Turn OFF previous Bonus Ladder lamp (unless it's the 10K light)
                if (gChaserIndex > 0) {
                    RPU_SetLampState(BonusLadderLamps[gChaserIndex - 1], 0);
                }
                RPU_SetLampState(BonusLadderLamps[gChaserIndex], 1);
                gChaserIndex++; // Move to the next bonus ladder light
            }
            
            // P1.6 - Turn ON the current Spinner lamp
            RPU_SetLampState(SpinnerAdvanceLamps[gSpinnerAdvancerCount % NUM_SPINNER_ADVANCE_LAMPS], 1);
            break;

        case ATTRACT_PHASE_2_ARC_SURGE:
            // Phase 2 Logic: Timed Instructional Sequence (4 steps total, using attractStep)
            if (attractStep == 1) {
                StopAttractModeLights(); // Clear lights from previous step
                // Step 1: Pulse Extra Ball Lane (1.5s)
                RPU_SetLampState(LAMP_EXTRA_BALL_LANE, 1, 0, 250); // Flash
                gAttractModeTimer = CurrentTime + 1500;
                attractStep = 2;
            } else if (attractStep == 2) {
                // Step 2: Bonus Ladder Chase (2.0s) - For simplicity, we'll just flash the multipliers
                RPU_SetLampState(LAMP_EXTRA_BALL_LANE, 0);
                RPU_SetLampState(LAMP_BONUS_DOUBLE, 1, 0, 200);
                RPU_SetLampState(LAMP_BONUS_TRIPLE, 1, 0, 200);
                RPU_SetLampState(LAMP_BONUS_QUINTUPLE, 1, 0, 200);
                gAttractModeTimer = CurrentTime + 2000;
                attractStep = 3;
            } else if (attractStep == 3) {
                // Step 3: Pulse Arc Surge Targets (1.5s)
                StopAttractModeLights();
                RPU_SetLampState(LAMP_SAUCER_EJECT, 1, 0, 250); // Flash
                gAttractModeTimer = CurrentTime + 1500;
                attractStep = 4;
            } else {
                // Step 4: Transition to Phase 3
                attractPhase = ATTRACT_PHASE_3_WAVE;
                gChaserIndex = 0; // Reset chaser for Phase 3
                gAttractModeTimer = CurrentTime + 10;
            }
            break;

        case ATTRACT_PHASE_3_WAVE:
            // Phase 3 Logic: Full Playfield Wave/Chaser
            gAttractModeTimer = CurrentTime + 50; // Very fast chaser
            
            if (gChaserIndex < NUM_ALL_FEATURE_LAMPS) {
                RPU_SetLampState(AllFeatureLamps[gChaserIndex], 1);
                if (gChaserIndex > 0) {
                    RPU_SetLampState(AllFeatureLamps[gChaserIndex - 1], 0);
                }
                gChaserIndex++;
            } else {
                // Loop completion - transition back to Phase 1
                StopAttractModeLights(); // This will reset all variables including attractPhase
            }
            break;
    }
}

void setup() {
  RPU_init(RPU_Callback_GameLogic);

  // --- One-Time Game Initialization (from LectronamoRecharged::Initialize) ---
  highScore = RPU_ReadULFromEEProm(ADDR_HIGH_SCORE, 100000); // Load high score with a default of 100,000
  byte ballsPerGameSetting = RPU_ReadByteFromEEProm(ADDR_BALLS_PER_GAME);
  ballsPerGame = (ballsPerGameSetting == 1) ? 5 : 3;
  
  RPU_SetupGameSwitches(NUM_SWITCHES, NUM_PRIORITY_SWITCHES, gameSwitchArray);
  
  // Reset all game state variables
  gameState = ATTRACT_MODE;
  credits = 0;
  currentScore = 0;
  currentBonus = 0;
  bonusMultiplier = 1;
  extraBallLit = false;
  isBallSaveActive = false;
  firstHitMade = false;
  arcSurgeActive = false;
  inAuditMode = false;
  threeBankCompleteCount = 0;
  fiveBankCompleteCount = 0;
  spinnerHitCount = 0;
  holdBonus = false;
  extraBalls = 0;
  thumperScoreIs1000 = false;
}

void loop() {
  RPU_loop();
}
