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
// Sound Definitions (SND_) - For Stern SB-100 Sound Board (Bitmask)
//----------------------------
#define SND_10_POINTS       (1 << 0) // Bit 0 for 10 points sound
#define SND_100_POINTS      (1 << 1) // Bit 1 for 100 points sound
#define SND_1000_POINTS     (1 << 2) // Bit 2 for 1000 points sound
#define SND_10000_POINTS    (1 << 3) // Bit 3 for 10000 points sound
#define SND_ADD_BONUS       (1 << 4) // Bit 4 for bonus advance sound
#define SND_POP_BUMPER      (1 << 5) // Bit 5 for pop bumper sound

//----------------------------
// Lamp Definitions (LAMP_)
//----------------------------
// Bonus Multiplier Lamps (Status & Next Target)
#define LAMP_BONUS_DOUBLE       22   // J3 Pin 22 (2X Status)
#define LAMP_BONUS_TRIPLE       13   // J3 Pin 13 (3X Status)
#define LAMP_BONUS_QUINTUPLE    10   // J3 Pin 10 (5X Status)
#define LAMP_BONUS_2X_NEXT      25   // J1 Pin 25 (Next target is 2X)
#define LAMP_BONUS_3X_NEXT      11   // J1 Pin 11 (Next target is 3X)
#define LAMP_BONUS_5X_NEXT      20   // J3 Pin 20 (Next target is 5X)

// Feature & High-Value Lamps
#define LAMP_SAUCER_EJECT       6    // J1 Pin 6 (Saucer/Arc Surge Targets - Single Pin Control)
#define LAMP_ADVANCE_BONUS_3    17   // J3 Pin 17 (Advance Bonus #3 - Used in Attract Mode Chase)
#define LAMP_EXTRA_BALL_LANE    18   // J3 Pin 18 (Extra Ball Lane - Used in Attract Mode & Gameplay)
#define LAMP_SPINNER            28   // J1 Pin 28 (Spinner Scores 1000 When Lit)
#define LAMP_SHOOT_AGAIN        27   // J1 Pin 27 (Shoot Again / Ball Save Indicator)

// Bonus Ladder Lamps (Visual Chase)
#define LAMP_BONUS_1000         18   // J1 Pin 18 
#define LAMP_BONUS_2000         1    // J3 Pin 1
#define LAMP_BONUS_3000         26   // J3 Pin 26
#define LAMP_BONUS_4000         1    // J3 Pin 1 (Shared Pin - Must be handled carefully)
#define LAMP_BONUS_5000         15   // J1 Pin 15
#define LAMP_BONUS_6000         9    // J3 Pin 9
#define LAMP_BONUS_7000         25   // J3 Pin 25
#define LAMP_BONUS_8000         12   // J3 Pin 12
#define LAMP_BONUS_9000         17   // J1 Pin 17
#define LAMP_BONUS_10000        8    // J1 Pin 8 

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
// II. GAME LOGIC & STATE DEFINITIONS
//================================================================

//----------------------------
// EEPROM Memory Map
//----------------------------
#define ADDR_BALLS_PER_GAME 0x0001
#define ADDR_SPECIAL_LIMIT  0x0002
#define ADDR_HIGH_SCORE     0x0005
#define ADDR_MAX_CREDITS_17 0x0011
#define ADDR_MAX_CREDITS_18 0x0012
#define ADDR_MAX_CREDITS_19 0x0013
#define ADDR_BONUS_COUNTDOWN_METHOD 0x27 // MPU Switch 24
#define ADDR_FREE_PLAY_ADJUSTMENT 0x28   // MPU Switch 25
#define ADDR_EXTRA_BALL_BYPASS 0x29      // MPU Switch 26 (Placeholder)
#define ADDR_SAUCER_LIGHT_PERSISTENCE 0x21 // MPU Switch 14 (Placeholder)
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
extern bool specialAwardedThisBall;
extern unsigned long highScore;
extern int credits;
extern unsigned long lastSwitchHitTime;

//================================================================
// II. GAME LOGIC & STATE DEFINITIONS
//================================================================

//----------------------------
// Scoring Constants (L for long to ensure compatibility)
//----------------------------
#define SCORE_DROP_TARGET_BASE      500L     // Base award for hitting ANY target
#define SCORE_POP_BUMPER            100L     // Standard 5-ball game score
#define SCORE_3BANK_COMPLETION      6000L
#define SCORE_5BANK_COMPLETION      10000L
#define SCORE_SPINNER_BASE          100L
#define SCORE_SPINNER_LIT           1000L
#define SCORE_SKILL_SHOT            15000L   // Custom Lite Rule
#define SCORE_ARC_SURGE_T1          25000L   // Custom Lite Rule (Switch 25)
#define SCORE_ARC_SURGE_SUPER       75000L   // Custom Lite Rule (Saucer 40)
#define SCORE_OUTLANE               3000L

//----------------------------
// Attract Mode & Game State
//----------------------------
#define TIME_BALL_SAVE_DURATION_MS 20000L
#define TIME_ARC_SURGE_COMBO_MS    8000L

// Attract Mode Phase Definitions
#define ATTRACT_PHASE_1_CLASSIC_FLOW    1
#define ATTRACT_PHASE_2_ARC_SURGE       2
#define ATTRACT_PHASE_3_WAVE            3

// General Game State Flags
#define FLAG_SKILL_SHOT_ACTIVE          (1 << 0)
#define FLAG_ARC_SURGE_ACTIVE           (1 << 1)
#define FLAG_ARC_SURGE_T1_HIT           (1 << 2)
#define FLAG_EXTRA_BALL_COLLECTED       (1 << 3)
#define FLAG_SIDE_LANE_LIT              (1 << 4) // Stationary Target made
#define FLAG_LEFT_RETURN_LANE_LIT       (1 << 5) // Rollover Button made
#endif // LECTRONAMO_H
