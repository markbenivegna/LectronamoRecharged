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
#define SW_RIGHT_OUTLANE            0
#define SW_LEFT_OUTLANE             1
#define SW_RIGHT_INLANE             2
#define SW_LEFT_INLANE              3
#define SW_TARGET_1_5BANK           4
#define SW_CREDIT_BUTTON            5
#define SW_TILT                     6
#define SW_OUTHOLE                  7
#define SW_COIN_3                   8
#define SW_COIN_1                   9
#define SW_COIN_2                  10
#define SW_STANDUP_TARGET          11
#define SW_TARGET_2_5BANK          12
#define SW_TARGET_3_3BANK          13
#define SW_ROLLOVER_BUTTON         14
#define SW_SLAM_TILT               15
#define SW_SPINNER                 16
#define SW_ADV_BONUS_300           17
#define SW_SCORE_10                18
// #define SW_                     19
#define SW_TARGET_3_5BANK          20
#define SW_TARGET_2_3BANK          21
#define SW_SAUCER                  22
#define SW_KICKER                  23
#define SW_ADV_BONUS_1000          24
// #define SW_                     25
// #define SW_                     26
// #define SW_                     27
#define SW_TARGET_4_5BANK          28
#define SW_TARGET_1_3BANK          29
// #define SW_                     30
// #define SW_                     31
#define SW_BOTTOM_POP              32
#define SW_RIGHT_POP               33
#define SW_LEFT_POP                34
// #define SW_                     35
#define SW_TARGET_5_5BANK          36
#define SW_RIGHT_SLING             37
#define SW_LEFT_SLING              38
// #define SW_                     39

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

#define LAMP_5_BANK_TARGET_SPECIAL      32 // Q1
#define LAMP_BONUS_MULTIPLIER_2X        36 // Q2
#define LAMP_SHOOT_AGAIN                40 // Q3
#define LAMP_SPINNER                    28 // Q4
#define LAMP_PLAYER_1                   52 // Q5
#define LAMP_1ST_PLAYER_UP              56 // Q6
// #define LAMP_UNUSED                  44 // Q7
#define LAMP_ADVANCE_BONUS_1            12 // Q8
// #define LAMP_UNUSED                  16 // Q9
// #define LAMP_UNUSED                  20 // Q10
#define LAMP_EXTRA_BALL                 24 // Q11
#define LAMP_BONUS_5K                   4 // Q12
#define LAMP_BONUS_9K                   8 // Q13
#define LAMP_BONUS_1K                   0 // Q14
#define LAMP_HIGHEST_SCORE              49 // Q15
#define LAMP_BALL_IN_PLAY               48 // Q16
#define LAMP_BONUS_MULTIPLIER_3X        37 // Q17
#define LAMP_PLAYER_2                   53 // Q18
#define LAMP_2ND_PLAYER_UP              57 // Q19
// #define LAMP_UNUSED                  30 // Q20
// #define LAMP_UNUSED                  45 // Q21
// #define LAMP_UNUSED                  21 // Q22
#define LAMP_MATCH                      41 // Q23
#define LAMP_ROLLOVER_BUTTON            32 // Q24
// #define LAMP_UNUSED                  3 // Q24
#define LAMP_SAUCER                     29 // Q25 (Also Left kickout lamp)
// #define LAMP_UNUSED                  25 // Q26
#define LAMP_BONUS_6K                   5 // Q27
#define LAMP_BONUS_10K                  9 // Q28
#define LAMP_BONUS_2K                   1 // Q29
#define LAMP_PLAYER_3                   54 // Q30
#define LAMP_3RD_PLAYER_UP              58 // Q31
// #define LAMP_UNUSED                  26 // Q32
#define LAMP_GAME_OVER                  50 // Q33
// #define LAMP_UNUSED                  17 // Q34
#define LAMP_ADVANCE_BONUS_2            13 // Q35
#define LAMP_BONUS_3K                   2 // Q36
// #define LAMP_UNUSED                  22 // Q37
#define LAMP_BONUS_7K                   6 // Q38
// #define LAMP_UNUSED                  46 // Q39
#define LAMP_DROP_TARGET_2X             42 // Q40
#define LAMP_BONUS_MULTIPLIER_5X        38 // Q41
#define LAMP_LEFT_RETURN                34 // Q42
#define LAMP_PLAYER_4                   55 // Q43
// #define LAMP_UNUSED                  10 // Q44
#define LAMP_4TH_PLAYER_UP              59 // Q45
#define LAMP_EXTRA_BALL_LANE            39 // Q46
#define LAMP_TILT                       51 // Q47
// #define LAMP_UNUSED                  18 // Q48
#define LAMP_ADVANCE_BONUS_3            14 // Q49
#define LAMP_BONUS_8K                   7 // Q50
// #define LAMP_UNUSED                  11 // Q51
#define LAMP_DROP_TARGET_3X             43 // Q52
// #define LAMP_UNUSED                  47 // Q53
#define LAMP_ADVANCE_BONUS_4            15 // Q54
// #define LAMP_UNUSED                  19 // Q55
#define LAMP_DROP_TARGET_5X             35 // Q56
#define LAMP_BONUS_4K                   3 // Q57
// #define LAMP_UNUSED                  31 // Q58
// #define LAMP_UNUSED                  27 // Q59
// #define LAMP_UNUSED                  23 // Q60

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
