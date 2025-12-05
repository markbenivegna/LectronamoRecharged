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
#define SCORE_3BANK_COMPLETION      15000L
#define SCORE_5BANK_COMPLETION      25000L
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
// Lamp Definitions (LAMP_)
//----------------------------
// Bonus Multiplier Lamps
#define LAMP_BONUS_DOUBLE       22
#define LAMP_BONUS_TRIPLE       13
#define LAMP_BONUS_QUINTUPLE    10
#define LAMP_BONUS_2X_NEXT      25
#define LAMP_BONUS_3X_NEXT      11
#define LAMP_BONUS_5X_NEXT      20

// Feature & High-Value Lamps
#define LAMP_SAUCER_EJECT       6
#define LAMP_ADVANCE_BONUS_3    17
#define LAMP_EXTRA_BALL_LANE    18
#define LAMP_SPINNER            28
#define LAMP_SHOOT_AGAIN        27

// Bonus Ladder Lamps
#define LAMP_BONUS_1000         18
#define LAMP_BONUS_2000         1
#define LAMP_BONUS_3000         26
#define LAMP_BONUS_4000         1
#define LAMP_BONUS_5000         15
#define LAMP_BONUS_6000         9
#define LAMP_BONUS_7000         25
#define LAMP_BONUS_8000         12
#define LAMP_BONUS_9000         17
#define LAMP_BONUS_10000        8

// Display & Cabinet Lamps
#define LAMP_BALL_IN_PLAY       122
#define LAMP_PLAYER_1           116
#define LAMP_PLAYER_UP_1        114
#define LAMP_MATCH              108
#define LAMP_HIGH_SCORE_TO_DATE 123
#define LAMP_PLAYER_2           120
#define LAMP_PLAYER_UP_2        115
#define LAMP_GAME_OVER          111
#define LAMP_PLAYER_3           106
#define LAMP_PLAYER_UP_3        102
#define LAMP_TILT               110
#define LAMP_PLAYER_4           107
#define LAMP_PLAYER_UP_4        101

//================================================================
// II. CONSTANTS AND GAME FLOW DEFINITIONS
//================================================================

// Game Flow States
#define HIGH_SCORE_CHECK 98

// Timings
#define TIME_MATCH_SEQUENCE_MS 3000
#define TIME_BALL_SAVE_DURATION_MS 15000
#define TIME_ARC_SURGE_COMBO_MS 5000

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
    HIGH_SCORE_CHECK, // State for checking and awarding high scores
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

//================================================================
// V. GENERAL GAME FLAGS
//================================================================
#define FLAG_SKILL_SHOT_ACTIVE          (1 << 0)
#define FLAG_ARC_SURGE_ACTIVE           (1 << 1)
#define FLAG_ARC_SURGE_T1_HIT           (1 << 2)
#define FLAG_EXTRA_BALL_COLLECTED       (1 << 3)
#define FLAG_SIDE_LANE_LIT              (1 << 4)
#define FLAG_LEFT_RETURN_LANE_LIT       (1 << 5)
#endif // LECTRONAMO_H
