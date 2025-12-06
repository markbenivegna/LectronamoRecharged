#ifndef LECTRONAMO_H
#define LECTRONAMO_H

#include <Arduino.h> // Defines byte for use in header file
#include "RPU.h"

//================================================================
// I. HARDWARE DEFINITIONS AND SCORING
//================================================================

// Score Constants
#define SCORE_DROP_TARGET_BASE      500L
#define SCORE_POP_BUMPER            100L
#define SCORE_OUTLANE               3000L
#define SCORE_SKILL_SHOT            5000L
#define SCORE_SPINNER_BASE          100L
#define SCORE_SPINNER_LIT           1000L
#define SCORE_3BANK_COMPLETION      6000L
#define SCORE_5BANK_COMPLETION      10000L
#define SCORE_ARC_SURGE_T1          10000L
#define SCORE_ARC_SURGE_SUPER       50000L

//----------------------------
// Switch Definitions (SW_)
//----------------------------
#define SW_RIGHT_OUTLANE 1 // J1 Pin 1
#define SW_RIGHT_RETURN_LANE 2 // J1 Pin 2
#define SW_LEFT_OUTLANE 3 // J1 Pin 3
#define SW_LEFT_RETURN_LANE 4 // J1 Pin 4
#define SW_TARGET_1_5BANK 5 // J1 Pin 5
#define SW_CREDIT_BUTTON 6 // J1 Pin 6
#define SW_TILT 7 // J1 Pin 7
#define SW_OUTHOLE 8 // J1 Pin 8
#define SW_COIN_3 9 // J1 Pin 9
#define SW_COIN_1 10 // J1 Pin 10
#define SW_COIN_2 11 // J1 Pin 11
#define SW_STANDUP_TARGET 12 // J1 Pin 12
#define SW_TARGET_2_5BANK 13 // J1 Pin 13
#define SW_TARGET_3_3BANK 14 // J1 Pin 14
#define SW_SPIN_TARGET 15 // J1 Pin 15
#define SW_SLAM_TILT 16 // J1 Pin 16
#define SW_ROLLOVER_BUTTON 17 // J1 Pin 17
#define SW_ADV_BONUS_300 18 // J1 Pin 18
#define SW_SCORE_10 19 // J1 Pin 19
#define SW_UNUSED_20 20 // J1 Pin 20
#define SW_TARGET_3_5BANK 21 // J2 Pin 1
#define SW_TARGET_2_3BANK 22 // J2 Pin 2
#define SW_UNUSED_23 23 // J2 Pin 3
#define SW_KICKER 24 // J2 Pin 4
#define SW_ADV_BONUS_1000 25 // J2 Pin 5
#define SW_UNUSED_26 26 // J2 Pin 6
#define SW_UNUSED_27 27 // J2 Pin 7
#define SW_UNUSED_28 28 // J2 Pin 8
#define SW_TARGET_4_5BANK 29 // J2 Pin 9
#define SW_TARGET_1_3BANK 30 // J2 Pin 10
#define SW_UNUSED_31 31 // J2 Pin 11
#define SW_UNUSED_32 32 // J2 Pin 12
#define SW_THUMPER_CENTER 33 // J2 Pin 13
#define SW_THUMPER_RIGHT 34 // J2 Pin 14
#define SW_THUMPER_LEFT 35 // J2 Pin 15
#define SW_UNUSED_36 36 // J2 Pin 16
#define SW_TARGET_5_5BANK 37 // J2 Pin 17
#define SW_RIGHT_SLINGSHOT 38 // J2 Pin 18
#define SW_LEFT_SLINGSHOT 39 // J2 Pin 19
#define SW_SAUCER 40 // J2 Pin 20

//----------------------------
// Solenoid Definitions (SOL_)
//----------------------------
#define SOL_LEFT_THUMPER 7 // Solenoid 7
#define SOL_RIGHT_THUMPER 8 // Solenoid 8
#define SOL_CENTER_THUMPER 9 // Solenoid 9
#define SOL_OUTHOLE 10 // Solenoid 10
#define SOL_RIGHT_SLINGSHOT 11 // Solenoid 11
#define SOL_KICKER 12 // Solenoid 12
#define SOL_DROP_TARGET_3BANK_RESET 13 // Solenoid 13
#define SOL_SAUCER 14 // Solenoid 14
#define SOL_DROP_TARGET_5BANK_RESET 15 // Solenoid 15
#define SOL_LEFT_SLINGSHOT 16 // Solenoid 16
#define SOL_KNOCKER 17 // Solenoid 17
#define SOL_FLIPPERS 18 // Solenoid 18
#define SOL_COIN_LOCKOUT 19 // Solenoid 19

//----------------------------
// Sound Definitions (SND_) - For Stern SB-100 Sound Board (Bitmask)
//----------------------------
#define SND_10_POINTS       (1 << 0)
#define SND_100_POINTS      (1 << 1)
#define SND_1000_POINTS     (1 << 2)
#define SND_10000_POINTS    (1 << 3)
#define SND_ADD_BONUS       (1 << 4)
#define SND_POP_BUMPER      (1 << 5)

//----------------------------
// Lamp Definitions (LAMP_) - Synchronized to Lectronamo Q-Connector Addresses
//----------------------------

// Bonus Multiplier Lamps
#define LAMP_BONUS_DOUBLE       2  // Q2
#define LAMP_BONUS_TRIPLE       17 // Q17
#define LAMP_BONUS_QUINTUPLE    41 // Q41
#define LAMP_BONUS_2X_NEXT      35 // Q35 (Advance Bonus #2)
#define LAMP_BONUS_3X_NEXT      49 // Q49 (Advance Bonus #3)
#define LAMP_BONUS_5X_NEXT      54 // Q54 (Advance Bonus #4)

// Feature & High-Value Lamps
#define LAMP_SAUCER_EJECT       25 // Q25
#define LAMP_EXTRA_BALL_LANE    46 // Q46
#define LAMP_ARC_SURGE_TARGET_1 42 // Q42 (Left Return Lamp - First target of the Arc Surge Combo)
#define LAMP_SPINNER            44 // Q44
#define LAMP_EXTRA_BALL         11 // Q11 (Used for Shoot Again light logic, but tied to Extra Ball)

// Bonus Ladder Lamps (1K through 10K)
// NOTE: These addresses MUST be sequentially correct for the chase routines (RunBonusLadderChase).
#define LAMP_BONUS_1000         14 // Q14
#define LAMP_BONUS_2000         29 // Q29
#define LAMP_BONUS_3000         36 // Q36
#define LAMP_BONUS_4000         57 // Q57
#define LAMP_BONUS_5000         12 // Q12
#define LAMP_BONUS_6000         27 // Q27
#define LAMP_BONUS_7000         38 // Q38
#define LAMP_BONUS_8000         50 // Q50
#define LAMP_BONUS_9000         13 // Q13
#define LAMP_BONUS_10000        28 // Q28

// Display & Cabinet Lamps
#define LAMP_BALL_IN_PLAY       16 // Q16
#define LAMP_PLAYER_1           5  // Q5
#define LAMP_PLAYER_2           18 // Q18
#define LAMP_PLAYER_3           30 // Q30
#define LAMP_PLAYER_4           43 // Q43
#define LAMP_PLAYER_UP_1        6  // Q6 (1st Player Up)
#define LAMP_PLAYER_UP_2        19 // Q19 (2nd Player Up)
#define LAMP_PLAYER_UP_3        31 // Q31 (3rd Player Up)
#define LAMP_PLAYER_UP_4        45 // Q45 (4th Player Up)
#define LAMP_MATCH              23 // Q23
#define LAMP_HIGH_SCORE_TO_DATE 15 // Q15
#define LAMP_GAME_OVER          33 // Q33
#define LAMP_TILT               47 // Q47
#define LAMP_SHOOT_AGAIN        3  // Q3

//================================================================
// II. CONSTANTS AND GAME FLOW DEFINITIONS
//================================================================

// Game Flow States
#define HIGH_SCORE_CHECK 98

// Timings
#define TIME_MATCH_SEQUENCE_MS 3000
#define TIME_BALL_SAVE_DURATION_MS 15000
#define TIME_ARC_SURGE_COMBO_MS 8000

// Attract Mode Phases
#define ATTRACT_PHASE_1_CLASSIC_FLOW 1
#define ATTRACT_PHASE_2_ARC_SURGE 2
#define ATTRACT_PHASE_3_WAVE 3

// General Game State Flags
#define FLAG_SKILL_SHOT_ACTIVE          (1 << 0)
#define FLAG_ARC_SURGE_ACTIVE           (1 << 1)
#define FLAG_ARC_SURGE_T1_HIT           (1 << 2)
#define FLAG_EXTRA_BALL_COLLECTED       (1 << 3)
#define FLAG_SIDE_LANE_LIT              (1 << 4)
#define FLAG_LEFT_RETURN_LANE_LIT       (1 << 5)

//================================================================
// III. EEPROM ADDRESSES (MPU ADJUSTMENTS)
//================================================================
#define ADDR_MAX_TILT_WARNINGS 10
#define ADDR_EXTRA_BALL_SCORE 11
#define ADDR_SPECIAL_SCORE 12
#define ADDR_HIGHSCORE_REPLAY_AWARD 13
#define ADDR_SAUCER_LIGHT_PERSISTENCE 14
#define ADDR_FREE_PLAY_ADJUSTMENT 15

// Standard RPU OS EEPROM Addresses
#define ADDR_BALLS_PER_GAME 0x0001
#define ADDR_SPECIAL_LIMIT  0x0002
#define ADDR_HIGH_SCORE     0x0005
#define ADDR_MAX_CREDITS_17 0x0011
#define ADDR_MAX_CREDITS_18 0x0012
#define ADDR_MAX_CREDITS_19 0x0013
#define ADDR_BONUS_COUNTDOWN_METHOD 0x27
#define ADDR_EXTRA_BALL_BYPASS 0x29

//================================================================
// IV. GLOBAL GAME STATE AND VARIABLES
//================================================================

enum GameState {
    ATTRACT_MODE,
    GAME_START,
    BALL_IN_PLAY,
    BONUS_COUNT,
    GAME_OVER,
    MATCH_MODE,
    AUDIT_MODE
};
extern GameState gameState;
extern long playerScores[4];
extern int currentBonus;
extern int bonusMultiplier;
extern bool extraBallLit;
extern unsigned long ballSaveStartTime;
extern bool isBallSaveActive;
extern bool firstHitMade;
extern unsigned long arcSurgeTimerStart;
extern bool arcSurgeActive;
extern int attractPhase;
extern int attractStep;
extern unsigned long selfTestTimerStart;
extern int selfTestPressCount;
extern bool inAuditMode;
extern int currentTestMode;
extern int threeBankCompleteCount;
extern bool threeTargetsDown[3];
extern int fiveBankCompleteCount;
extern bool fiveTargetsDown[5];
extern int spinnerHitCount;
extern bool holdBonus;
extern int extraBalls;
extern int ball;
extern int player;
extern int ballsPerGame;
extern bool specialAwardedThisBall;
extern unsigned long highScore;
extern int credits;
extern unsigned long lastSwitchHitTime;
extern byte MaxTiltWarnings;
extern byte NumTiltWarnings;
extern long ExtraBallScoreValue;
extern long SpecialScoreValue;
extern byte AwardHighscoreNumReplays;
#endif // LECTRONAMO_H
