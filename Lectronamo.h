#ifndef LECTRONAMO_H
#define LECTRONAMO_H

//================================================================
// I. HARDWARE DEFINITIONS (SWITCHES, LAMPS, SOLENOIDS)
//================================================================

#include <stdint.h>

//----------------------------
// Switch Definitions (SW_)
//----------------------------
#define SW_RIGHT_OUTLANE 1
#define SW_RIGHT_RETURN_LANE 2
#define SW_LEFT_OUTLANE 3
#define SW_LEFT_RETURN_LANE 4
#define SW_TARGET_1_5BANK 5
#define SW_CREDIT_BUTTON 6
#define SW_TILT 7
#define SW_OUTHOLE 8
#define SW_COIN_3 9
#define SW_COIN_1 10
#define SW_COIN_2 11
#define SW_STANDUP_TARGET 12
#define SW_TARGET_2_5BANK 13
#define SW_TARGET_3_3BANK 14
#define SW_SPIN_TARGET 15
#define SW_SLAM_TILT 16
#define SW_ROLLOVER_BUTTON 17
#define SW_ADV_BONUS_300 18
#define SW_SCORE_10 19
#define SW_UNUSED_20 20
#define SW_TARGET_3_5BANK 21
#define SW_TARGET_2_3BANK 22
#define SW_UNUSED_23 23
#define SW_KICKER 24
#define SW_ADV_BONUS_1000 25
#define SW_UNUSED_26 26
#define SW_UNUSED_27 27
#define SW_UNUSED_28 28
#define SW_TARGET_4_5BANK 29
#define SW_TARGET_1_3BANK 30
#define SW_UNUSED_31 31
#define SW_UNUSED_32 32
#define SW_THUMPER_CENTER 33
#define SW_THUMPER_RIGHT 34
#define SW_THUMPER_LEFT 35
#define SW_UNUSED_36 36
#define SW_TARGET_5_5BANK 37
#define SW_RIGHT_SLINGSHOT 38
#define SW_LEFT_SLINGSHOT 39
#define SW_SAUCER 40

//----------------------------
// Solenoid Definitions (SOL_)
//----------------------------
#define SOL_LEFT_THUMPER 7
#define SOL_RIGHT_THUMPER 8
#define SOL_CENTER_THUMPER 9
#define SOL_OUTHOLE 10
#define SOL_RIGHT_SLINGSHOT 11
#define SOL_KICKER 12
#define SOL_DROP_TARGET_3BANK_RESET 13
#define SOL_SAUCER 14
#define SOL_DROP_TARGET_5BANK_RESET 15
#define SOL_LEFT_SLINGSHOT 16
#define SOL_KNOCKER 17
#define SOL_FLIPPERS 18
#define SOL_COIN_LOCKOUT 19

//----------------------------
// Lamp Definitions (LAMP_)
//----------------------------
#define LAMP_COLLECT_3000 26
#define LAMP_COLLECT_7000 25
#define LAMP_ADV_BONUS_3 17
#define LAMP_LEFT_RETURN_9000 21
#define LAMP_5X 20
#define LAMP_2X 22
#define LAMP_COLLECT_4000 1
#define LAMP_COLLECT_8000 12
#define LAMP_ADV_BONUS_4 11
#define LAMP_QUINTUPLE 10
#define LAMP_EXTRA_BALL 18
#define LAMP_3X 13
#define LAMP_BALL_IN_PLAY 122
#define LAMP_PLAYER_1 116
#define LAMP_PLAYER_UP_1 114
#define LAMP_MATCH 108
#define LAMP_HIGH_SCORE_TO_DATE 123
#define LAMP_PLAYER_2 120
#define LAMP_PLAYER_UP_2 115
#define LAMP_GAME_OVER 111
#define LAMP_PLAYER_3 106
#define LAMP_PLAYER_UP_3 102
#define LAMP_TILT 110
#define LAMP_PLAYER_4 107
#define LAMP_PLAYER_UP_4 101

//================================================================
// II. GAME LOGIC & STATE DEFINITIONS
//================================================================

//----------------------------
// Operator Adjustments
//----------------------------
#define ADJ_SKILL_SHOT_ENABLED true
#define ADJ_SKILL_SHOT_POINTS 5000
#define ADJ_SKILL_SHOT_TIMEOUT_MS 0
#define ADJ_BALL_SAVE_TIME_SEC 20
#define ADJ_ARC_SURGE_COMBO_TIME_MS 5000
#define ADJ_SPINNER_MODE_TIME_SEC 30
#define ADJ_POP_MODE_TIME_SEC 30
#define ADJ_DROP_MODE_TIME_SEC 60
#define ADJ_HURRY_UP_TIME_SEC 15
#define ADJ_MINI_WIZARD_RETRIES true
#define ADJ_MINI_WIZARD_SCORE 100000
#define ADJ_WIZARD_MODE_SCORE 500000
#define ADJ_WIZARD_MODE_RETRIES true
#define ODDS_LIGHT_EXTRA_BALL 5
#define ODDS_ADVANCE_POPS 20
#define ODDS_BIG_POINTS 5
#define ODDS_PUNY_POINTS 40
#define ODDS_ADVANCE_MULTIPLIER 10
#define ODDS_HOLD_BONUS 15
#define ODDS_TILT_ENDS_GAME 5
#define ADJ_MAX_EXTRA_BALLS_PER_BALL 1
#define ADJ_BONUS_CAP_POINTS 30000
#define ADJ_BONUS_HOLD_ENABLED true

//----------------------------
// EEPROM Memory Map
//----------------------------
#define ADDR_BALLS_PER_GAME 0x0001
#define ADDR_SPECIAL_LIMIT  0x0002
#define ADDR_HIGH_SCORE     0x0005
#define ADDR_MAX_CREDITS_17 0x0011
#define ADDR_MAX_CREDITS_18 0x0012
#define ADDR_MAX_CREDITS_19 0x0013
//----------------------------
// Game State Definitions
//----------------------------
enum GameState {
    ATTRACT_MODE,
    GAME_START,
    BALL_IN_PLAY,
    BONUS_COUNT,
    GAME_OVER,
    AUDIT_MODE
};

//----------------------------
// Global Game Variables
//----------------------------
extern GameState gameState;
extern long currentScore;
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
extern bool thumperScoreIs1000;
extern bool specialAwardedThisBall;
extern unsigned long highScore;
extern int credits;

#endif // LECTRONAMO_H
